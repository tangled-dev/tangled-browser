
#include "chrome/browser/ui/webui/millix/untrusted_millix_app_config.h"
#include "chrome/browser/ui/webui/millix/untrusted_millix_app.h"
#include "chrome/common/url_constants.h"
#include "chrome/common/webui_url_constants.h"

UntrustedMillixAppConfig::UntrustedMillixAppConfig()
    : WebUIConfig(content::kChromeUIUntrustedScheme, chrome::kChromeUIMillixUntrustedAppHost) {}
    
std::unique_ptr<content::WebUIController>
UntrustedMillixAppConfig::CreateWebUIController(
    content::WebUI* web_ui) {
  return std::make_unique<UntrustedMillixAppUI>(web_ui);
}