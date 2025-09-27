#include "chrome/browser/ui/webui/millix/untrusted_millix_ws.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/common/url_constants.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/browser_resources.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"

UntrustedMillixWSUI::UntrustedMillixWSUI(content::WebUI* web_ui)
    : ui::UntrustedWebUIController(web_ui) {
  // Set up the chrome-untrusted://milix-ws source.
  content::WebUIDataSource* untrusted_source =
      content::WebUIDataSource::Create(chrome::kChromeUIMillixWSURL);

  // Add required resources.
  untrusted_source->AddResourcePath("millix_ws.js", IDR_MILLIX_WS_JS);
  untrusted_source->AddResourcePath("config.js", IDR_MILLIX_BAR_CONFIG_JS);
  untrusted_source->SetDefaultResource(IDR_MILLIX_WS_HTML);

  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc, "connect-src wss: https:;");
  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc, "script-src chrome-untrusted://resources https: 'self' 'unsafe-inline';");
  untrusted_source->AddFrameAncestor(GURL(chrome::kChromeUIMillixBarURL));

  Profile* profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource::Add(profile, untrusted_source);
}