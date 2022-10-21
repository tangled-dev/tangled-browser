#include "chrome/browser/ui/webui/millix/millix_app.h"

#include "base/bind.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/millix/millix_bar_view.h"
#include "chrome/browser/ui/webui/millix/millix_bar.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/browser_resources.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui_data_source.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"
#include "ui/native_theme/native_theme.h"

namespace views {
class MillixBarView;
}

MillixAppUI::MillixAppMessageHandler::MillixAppMessageHandler() {}

MillixAppUI::MillixAppMessageHandler::~MillixAppMessageHandler() {}

void MillixAppUI::MillixAppMessageHandler::HandleInitializeMessage(
    const base::Value::List& args) {
  auto* profile = Profile::FromWebUI(web_ui());
  Browser* browser = chrome::FindTabbedBrowser(profile, false);
  auto* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
  auto* millix_bar = (MillixBarUI*)(browser_view->GetMillixBarView()->GetWebContents()->GetWebUI()->GetController());
  base::DictionaryValue apiConfig;
  apiConfig.SetStringKey("node_id", millix_bar->GetNodeId());
  apiConfig.SetStringKey("node_signature", millix_bar->GetNodeSignature());
  AllowJavascript();
  CallJavascriptFunction("onLoadNodeApiConfig", apiConfig);
}

void MillixAppUI::MillixAppMessageHandler::UpdateMillixWallet(
    const base::Value::List& args) {
  auto* profile = Profile::FromWebUI(web_ui());
  Browser* browser = chrome::FindTabbedBrowser(profile, false);
  auto* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
  auto* web_contents = browser_view->GetMillixBarView()->GetWebContents();
  web_contents->GetWebUI()->ProcessWebUIMessage(
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
  content::WebUIDataSource* html_source =
      content::WebUIDataSource::Create(chrome::kChromeUIMillixAppHost);

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

  Profile* profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource::Add(profile, html_source);
}

MillixAppUI::~MillixAppUI() {}