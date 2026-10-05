#include "chrome/browser/ui/webui/millix/millix_bar.h"

#include "base/functional/bind.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_navigator_params.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/singleton_tabs.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/browser_resources.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/browser/browser_main_loop.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"
#include "ui/native_theme/native_theme.h"

namespace content {
class WebContents;
class BrowserContext;
class BrowserMainLoop;
}  // namespace content

MillixBarUI::MillixBarMessageHandler::MillixBarMessageHandler() {}

MillixBarUI::MillixBarMessageHandler::~MillixBarMessageHandler() {}

void MillixBarUI::MillixBarMessageHandler::HandleInitializeMessage(
  const base::ListValue& args) {
  base::DictValue apiConfig;
  apiConfig.Set("node_id", this->node_id);
  apiConfig.Set("node_signature", this->node_signature);
  base::DictValue message;
  message.Set("type", "api_config_update");
  message.Set("config", std::move(apiConfig));
  AllowJavascript();
  CallJavascriptFunction("millix_bar.onMillixBarMessage", message);
}

void MillixBarUI::MillixBarMessageHandler::UpdateMillixWallet(
    const base::ListValue& args) {
  AllowJavascript();
  if (args.empty() || !args[0].is_dict()) {
    return;
  }
  auto& message = args[0];
  const std::string* message_type = message.GetDict().FindString("type");
  if (message_type && *message_type == "wallet_update_state") {
    auto* profile = Profile::FromWebUI(web_ui());
    Browser* browser = chrome::FindTabbedBrowser(profile, false);
    BrowserView* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
    browser_view->GetMillixBarView()->SetVisible(true);
  }

  if (message.GetDict().FindString("from_bar")) {
    return;
  }

  CallJavascriptFunction("millix_bar.onMillixBarMessage", message);
}

void MillixBarUI::MillixBarMessageHandler::RestarMillixNode(const base::ListValue& args) {
  content::BrowserMainLoop::GetInstance()->CreateMillixNode();
}

void MillixBarUI::MillixBarMessageHandler::ShowMillixWallet(
    const base::ListValue& args) {
  auto* profile = Profile::FromWebUI(web_ui());
  Browser* browser = chrome::FindTabbedBrowser(profile, false);
  if (!browser)
    browser = Browser::Create(Browser::CreateParams(profile, true));
  auto* const contents = browser->tab_strip_model()->GetActiveWebContents();
  if (contents) {
    contents->Focus();
  }

  std::string page;
  if (!args.empty()) {
    page = args[0].GetString();
  }

  if (page.empty()) {
    NavigateParams params(GetSingletonTabNavigateParams(
        browser, GURL(chrome::kChromeUIMillixAppURL)));
    params.path_behavior = NavigateParams::IGNORE_AND_NAVIGATE;
    ShowSingletonTabOverwritingNTP(&params);
  } else if (page == "refresh") {
    int nTabs = browser->tab_strip_model()->count();
    for (int i = 0; i < nTabs; i++) {
      content::WebContents* webContents =
          browser->tab_strip_model()->GetWebContentsAt(i);
      std::string_view host = webContents->GetURL().host();
      std::string_view path = webContents->GetURL().path();
      if (host == chrome::kChromeUIMillixAppHost && path == "/") {
        webContents->GetPrimaryMainFrame()->Reload();
      }
    }
  } else if (page == "new_tab") {
    std::string newTabURL = args[1].GetString();
    NavigateParams params(GetSingletonTabNavigateParams(browser, GURL(newTabURL)));
    params.path_behavior = NavigateParams::IGNORE_AND_NAVIGATE;
    ShowSingletonTabOverwritingNTP(&params);
  } else {
    std::string url = base::StringPrintf("%s/%s", chrome::kChromeUIMillixAppURL,
                                         page.c_str());
    NavigateParams params(GetSingletonTabNavigateParams(browser, GURL(url)));
    params.path_behavior = NavigateParams::IGNORE_AND_NAVIGATE;
    ShowSingletonTabOverwritingNTP(&params);
  }
}

void MillixBarUI::MillixBarMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "initialize",
      base::BindRepeating(&MillixBarMessageHandler::HandleInitializeMessage,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "showMillixWallet", base::BindRepeating(&MillixBarMessageHandler::ShowMillixWallet,
                                     base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "updateMillixWallet",
      base::BindRepeating(&MillixBarMessageHandler::UpdateMillixWallet,
                 base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "restartMillixNode",
      base::BindRepeating(&MillixBarMessageHandler::RestarMillixNode,
                 base::Unretained(this)));
}

MillixBarUI::MillixBarUI(content::WebUI* web_ui)
    : ui::MojoWebUIController(web_ui, true) {
  // Set up the tangled://milix-bar source.
  Profile* profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* html_source =
      content::WebUIDataSource::CreateAndAdd(profile,
                                             chrome::kChromeUIMillixBarHost);

  // Add required resources.
  html_source->AddResourcePath("millix_bar.css", IDR_MILLIX_BAR_CSS);
  html_source->AddResourcePath("millix_bar.js", IDR_MILLIX_BAR_JS);
  html_source->AddResourcePath("config.js", IDR_MILLIX_BAR_CONFIG_JS);
  html_source->AddResourcePath("vendor/moment.min.js", IDR_MILLIX_BAR_MOMENTJS);
  html_source->AddResourcePath("vendor/jquery.js", IDR_MILLIX_BAR_JQUERY);
  html_source->AddResourcePath("vendor/jquery.nicescroll.js",
                               IDR_MILLIX_BAR_JQUERY_NICE_SCROLL);
  html_source->AddResourcePath("deposit.mp3", IDR_MILLIX_APP_DEPOSIT_MP3);
  html_source->AddBoolean(
      "is_dark_theme",
      ui::NativeTheme::GetInstanceForWeb()->preferred_color_scheme() ==
          ui::NativeTheme::PreferredColorScheme::kDark);
  html_source->UseStringsJs();
  html_source->SetDefaultResource(IDR_MILLIX_BAR_HTML);

  html_source->DisableTrustedTypesCSP();

  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc,
      "script-src tangled://resources 'self' 'unsafe-inline';");

  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ChildSrc,
      base::StringPrintf("child-src %s;", chrome::kChromeUIMillixWSURL));

  html_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::FrameSrc,
      base::StringPrintf("frame-src %s;", chrome::kChromeUIMillixWSURL));

  auto message_handler_ = std::make_unique<
      MillixBarMessageHandler>();  //(new MillixBarMessageHandler());
  this->message_handler = message_handler_.get();
  web_ui->AddMessageHandler(std::move(message_handler_));
  web_ui->AddRequestableScheme(content::kChromeUIUntrustedScheme);


  ReadNodeConfig();
}

MillixBarUI::~MillixBarUI() {}

void MillixBarUI::ReadNodeConfig() {
  base::FilePath file_path;
  base::PathService::Get(base::DIR_HOME, &file_path);
  file_path = file_path.AppendASCII("millix-tangled").AppendASCII("node.json");
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(
          [](const base::FilePath& path) -> std::optional<std::string> {
            std::string contents;
            if (!base::ReadFileToString(path, &contents) || contents.empty()) {
              return std::nullopt;
            }
            return contents;
          },
          file_path),
      base::BindOnce(&MillixBarUI::OnNodeConfigRead,
                     weak_factory_.GetWeakPtr()));
}

void MillixBarUI::OnNodeConfigRead(std::optional<std::string> json) {
  std::optional<base::Value> node_config;
  if (json) {
    node_config = base::JSONReader::Read(*json, base::JSON_PARSE_RFC);
  }
  const std::string* node_id = nullptr;
  const std::string* node_signature = nullptr;
  if (node_config && node_config->is_dict()) {
    node_id = node_config->GetDict().FindString("node_id");
    node_signature = node_config->GetDict().FindString("node_signature");
  }

  if (!node_id || !node_signature || node_id->empty() ||
      node_signature->empty()) {
    // The node writes node.json once it has started; retry until it exists.
    VLOG(1) << "millix node config not available yet, retrying";
    content::GetUIThreadTaskRunner({})->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&MillixBarUI::ReadNodeConfig,
                       weak_factory_.GetWeakPtr()),
        base::Seconds(1));
    return;
  }

  this->message_handler->node_id = *node_id;
  this->message_handler->node_signature = *node_signature;
  OnUpdateNodeApiConfig();
}

void MillixBarUI::OnUpdateNodeApiConfig() {
  base::DictValue apiConfig;
  apiConfig.Set("node_id", this->message_handler->node_id);
  apiConfig.Set("node_signature", this->message_handler->node_signature);
  base::DictValue message;
  message.Set("type", "api_config_update");
  message.Set("config", std::move(apiConfig));
  base::ListValue args;
  args.Append(std::move(message));
  web_ui()->ProcessWebUIMessage(GURL(chrome::kChromeUIMillixBarURL),
                                "updateMillixWallet", std::move(args));
}

std::string MillixBarUI::GetNodeId() const {
  return this->message_handler->node_id;
}

std::string MillixBarUI::GetNodeSignature() const {
  return this->message_handler->node_signature;
}