#include "chrome/browser/ui/webui/millix/millix_bar.h"

#include "base/bind.h"
#include "base/json/json_reader.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
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
  const base::Value::List& args) {
  base::DictionaryValue apiConfig;
  apiConfig.SetStringKey("node_id", this->node_id);
  apiConfig.SetStringKey("node_signature", this->node_signature);
  base::DictionaryValue message;
  message.SetStringKey("type", "api_config_update");
  message.Set(
      "config", std::make_unique<base::DictionaryValue>(std::move(apiConfig)));
  AllowJavascript();
  CallJavascriptFunction("millix_bar.onMillixBarMessage", message);
}

void MillixBarUI::MillixBarMessageHandler::UpdateMillixWallet(
    const base::Value::List& args) {
  AllowJavascript();
  auto& message = args[0];
  const std::string* message_type = message.FindStringPath("type");
  if (*message_type == "wallet_update_state") {
    auto* profile = Profile::FromWebUI(web_ui());
    Browser* browser = chrome::FindTabbedBrowser(profile, false);
    BrowserView* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
    browser_view->GetMillixBarView()->SetVisible(true);
  }

  if (message.FindStringPath("from_bar")) {
    return;
  }

  CallJavascriptFunction("millix_bar.onMillixBarMessage", message);
}

void MillixBarUI::MillixBarMessageHandler::RestarMillixNode(const base::Value::List& args) {
  content::BrowserMainLoop::GetInstance()->CreateMillixNode();
}

void MillixBarUI::MillixBarMessageHandler::ShowMillixWallet(
    const base::Value::List& args) {
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
    ShowSingletonTabOverwritingNTP(browser, &params);
  } else if (page == "refresh") {
    int nTabs = browser->tab_strip_model()->GetTabCount();
    for (int i = 0; i < nTabs; i++) {
      content::WebContents* webContents =
          browser->tab_strip_model()->GetWebContentsAt(i);
      std::string host = webContents->GetURL().host();
      std::string path = webContents->GetURL().path();
      if (host == chrome::kChromeUIMillixAppHost && path == "/") {
        webContents->GetPrimaryMainFrame()->Reload();
      }
    }
  } else if (page == "new_tab") {
    std::string newTabURL = args[1].GetString();
    NavigateParams params(GetSingletonTabNavigateParams(browser, GURL(newTabURL)));
    params.path_behavior = NavigateParams::IGNORE_AND_NAVIGATE;
    ShowSingletonTabOverwritingNTP(browser, &params);
  } else {
    std::string url = base::StringPrintf("%s/%s", chrome::kChromeUIMillixAppURL,
                                         page.c_str());
    NavigateParams params(GetSingletonTabNavigateParams(browser, GURL(url)));
    params.path_behavior = NavigateParams::IGNORE_AND_NAVIGATE;
    ShowSingletonTabOverwritingNTP(browser, &params);
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
  content::WebUIDataSource* html_source =
      content::WebUIDataSource::Create(chrome::kChromeUIMillixBarHost);

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
      ui::NativeTheme::GetInstanceForWeb()->ShouldUseDarkColors());
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

  Profile* profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource::Add(profile, html_source);

  content::GetIOThreadTaskRunner({})->PostTask(
      FROM_HERE,
      base::BindOnce(&MillixBarUI::ReadNodeConfig, base::Unretained(this)));
}

MillixBarUI::~MillixBarUI() {}

void MillixBarUI::ReadNodeConfig() {
  base::FilePath file_path;
  base::PathService::Get(base::DIR_HOME, &file_path);
  file_path = file_path.AppendASCII("millix-tangled").AppendASCII("node.json");
  LOG(ERROR) << "reading node config from " << file_path.value().c_str();
  this->node_config_file_reader = storage::FileStreamReader::CreateForLocalFile(
      base::ThreadPool::CreateTaskRunner(
          {base::MayBlock(), base::TaskPriority::USER_VISIBLE}),
      file_path, 0, base::Time());
  this->node_config_file_reader->GetLength(
      base::BindOnce(&MillixBarUI::ReadNodeConfigJson, base::Unretained(this)));
}

void MillixBarUI::ReadNodeConfigJson(int64_t size = -1) {
  LOG(ERROR) << "reading node config. size: " << size;
  if (size <= 0) {
    // retry later
    content::GetIOThreadTaskRunner({})->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&MillixBarUI::ReadNodeConfig, base::Unretained(this)),
        base::Milliseconds(1000));
    return;
  }
  this->node_config_buffer = base::MakeRefCounted<net::IOBufferWithSize>(size);
  int rv = this->node_config_file_reader->Read(
      this->node_config_buffer.get(), size,
      base::BindOnce(&MillixBarUI::OnReadNodeConfigJson,
                     base::Unretained(this)));
  if (rv > 0) {
    this->OnReadNodeConfigJson(rv);
  }
}

void MillixBarUI::OnReadNodeConfigJson(int result) {
  char* data = this->node_config_buffer->data();
  auto json_data = base::StringPiece(data).substr(0, result);
  LOG(ERROR) << "reading node config. json (" << result << ") "
             << json_data;
  auto node_config_json =
      base::JSONReader::Read(json_data);

  if (node_config_json.has_value()) {
    std::string* node_id = node_config_json->FindStringKey("node_id");
    std::string* node_signature =
        node_config_json->FindStringKey("node_signature");

    if (!node_id || !node_signature || node_id->empty() ||
        node_signature->empty()) {
      LOG(ERROR) << "reading node config. invalid configs! ";
      this->ReadNodeConfigJson();
      return;
    }

    LOG(ERROR) << "reading node config. id " << *node_id << " signature "
               << *node_signature;
    this->message_handler->node_id = *node_id;
    this->message_handler->node_signature = *node_signature;
    content::GetUIThreadTaskRunner({})->PostTask(
        FROM_HERE, base::BindOnce(&MillixBarUI::OnUpdateNodeApiConfig,
                                  base::Unretained(this)));
  } else {
    LOG(ERROR) << "reading node config. invalid json";
    // retry later
    content::GetIOThreadTaskRunner({})->PostDelayedTask(
        FROM_HERE,
        base::BindOnce(&MillixBarUI::ReadNodeConfig, base::Unretained(this)),
        base::Milliseconds(1000));
  }
}

void MillixBarUI::OnUpdateNodeApiConfig() {
  LOG(ERROR) << "sending node config. id " << this->message_handler->node_id
             << " signature " << this->message_handler->node_signature;
  base::DictionaryValue apiConfig;
  apiConfig.SetStringKey("node_id", this->message_handler->node_id);
  apiConfig.SetStringKey("node_signature",
                         this->message_handler->node_signature);
  base::DictionaryValue message;
  message.SetStringKey("type", "api_config_update");
  message.Set(
      "config", std::make_unique<base::DictionaryValue>(std::move(apiConfig)));
  base::Value::List args;
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