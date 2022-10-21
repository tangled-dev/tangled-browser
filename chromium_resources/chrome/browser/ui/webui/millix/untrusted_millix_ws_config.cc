
#include "chrome/browser/ui/webui/millix/untrusted_millix_ws_config.h"
#include "chrome/browser/ui/webui/millix/untrusted_millix_ws.h"
#include "chrome/common/url_constants.h"
#include "chrome/common/webui_url_constants.h"

UntrustedMillixWSConfig::UntrustedMillixWSConfig()
    : WebUIConfig(content::kChromeUIUntrustedScheme, chrome::kChromeUIMillixWSHost) {}
    
std::unique_ptr<content::WebUIController>
UntrustedMillixWSConfig::CreateWebUIController(
    content::WebUI* web_ui) {
  return std::make_unique<UntrustedMillixWSUI>(web_ui);
}