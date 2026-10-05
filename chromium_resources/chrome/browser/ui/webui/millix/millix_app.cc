#include "chrome/browser/ui/webui/millix/millix_app.h"

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/millix/millix_bar_view.h"
#include "chrome/browser/ui/webui/millix/millix_bar.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/browser_resources.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"
#include "ui/native_theme/native_theme.h"

namespace views {
class MillixBarView;
}

MillixAppUI::MillixAppMessageHandler::MillixAppMessageHandler() {}

MillixAppUI::MillixAppMessageHandler::~MillixAppMessageHandler() {}

content::WebContents*
MillixAppUI::MillixAppMessageHandler::GetMillixBarContents() {
  Browser* browser =
      chrome::FindTabbedBrowser(Profile::FromWebUI(web_ui()), false);
  if (!browser) {
    VLOG(1) << "millix app: no tabbed browser";
    return nullptr;
  }
  BrowserView* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
  if (!browser_view || !browser_view->GetMillixBarView()) {
    VLOG(1) << "millix app: no millix bar view";
    return nullptr;
  }
  content::WebContents* contents =
      browser_view->GetMillixBarView()->GetWebContents();
  if (!contents || !contents->GetWebUI() ||
      contents->GetLastCommittedURL().host() !=
          chrome::kChromeUIMillixBarHost) {
    VLOG(1) << "millix app: millix bar webui not ready, url="
            << (contents ? contents->GetLastCommittedURL().spec() : "none");
    return nullptr;
  }
  return contents;
}

void MillixAppUI::MillixAppMessageHandler::HandleInitializeMessage(
    const base::ListValue& args) {
  AllowJavascript();
  // Drop retries from an earlier initialize; the page asks again on reload.
  weak_factory_.InvalidateWeakPtrs();
  SendNodeApiConfigWhenReady();
}

void MillixAppUI::MillixAppMessageHandler::SendNodeApiConfigWhenReady() {
  if (!IsJavascriptAllowed()) {
    VLOG(1) << "millix app: javascript not allowed, dropping config send";
    return;
  }
  content::WebContents* bar_contents = GetMillixBarContents();
  auto* millix_bar =
      bar_contents
          ? static_cast<MillixBarUI*>(bar_contents->GetWebUI()->GetController())
          : nullptr;
  if (!millix_bar || millix_bar->GetNodeId().empty() ||
      millix_bar->GetNodeSignature().empty()) {
    // The node writes node.json once it has started; at browser startup a
    // restored tangled://millix tab can get here first.
    VLOG(1) << "millix app: node config not ready, retrying";
    content::GetUIThreadTaskRunner({})->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&MillixAppMessageHandler::SendNodeApiConfigWhenReady,
                       weak_factory_.GetWeakPtr()),
        base::Seconds(1));
    return;
  }
  base::DictValue apiConfig;
  apiConfig.Set("node_id", millix_bar->GetNodeId());
  apiConfig.Set("node_signature", millix_bar->GetNodeSignature());
  VLOG(1) << "millix app: sending node config to page";
  CallJavascriptFunction("onLoadNodeApiConfig", apiConfig);
}

void MillixAppUI::MillixAppMessageHandler::OnJavascriptDisallowed() {
  weak_factory_.InvalidateWeakPtrs();
}

void MillixAppUI::MillixAppMessageHandler::UpdateMillixWallet(
    const base::ListValue& args) {
  content::WebContents* bar_contents = GetMillixBarContents();
  if (!bar_contents) {
    return;
  }
  bar_contents->GetWebUI()->ProcessWebUIMessage(
      GURL(chrome::kChromeUIMillixBarURL), "updateMillixWallet", args.Clone());
}

void MillixAppUI::MillixAppMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "updateMillixWallet",
      base::BindRepeating(&MillixAppMessageHandler::UpdateMillixWallet,
                 base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "initialize",
      base::BindRepeating(&MillixAppMessageHandler::HandleInitializeMessage,
                 base::Unretained(this)));
}

MillixAppUI::MillixAppUI(content::WebUI* web_ui)
    : ui::MojoWebUIController(web_ui, true) {
  // Set up the tangled://milix source.
  Profile* profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* html_source =
      content::WebUIDataSource::CreateAndAdd(profile,
                                             chrome::kChromeUIMillixAppHost);

  // Add required resources.

  html_source->AddResourcePath("favicon.ico", IDR_MILLIX_APP_FAVICON_HTML);
  html_source->AddResourcePath("static/js/main.js", IDR_MILLIX_APP_JS);
  html_source->UseStringsJs();
  html_source->SetDefaultResource(IDR_MILLIX_APP_HTML);

  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc,
      "script-src tangled://resources 'self' 'unsafe-inline';");

  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ChildSrc,
      base::StringPrintf("child-src %s;",
                         chrome::kChromeUIMillixUntrustedAppURL));

  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::FrameSrc,
      base::StringPrintf("frame-src %s;",
                         chrome::kChromeUIMillixUntrustedAppURL));

  html_source->DisableTrustedTypesCSP();

  web_ui->AddRequestableScheme(content::kChromeUIUntrustedScheme);
  web_ui->AddMessageHandler(std::make_unique<MillixAppMessageHandler>());

}

MillixAppUI::~MillixAppUI() {}