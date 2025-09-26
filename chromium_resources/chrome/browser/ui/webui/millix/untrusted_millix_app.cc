#include "chrome/browser/ui/webui/millix/untrusted_millix_app.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/common/url_constants.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/browser_resources.h"
#include "chrome/grit/generated_resources.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"

UntrustedMillixAppUI::UntrustedMillixAppUI(content::WebUI* web_ui)
    : ui::UntrustedWebUIController(web_ui) {
  // Set up the chrome-untrusted://milix source.
  content::WebUIDataSource* untrusted_source =
      content::WebUIDataSource::Create(chrome::kChromeUIMillixUntrustedAppURL);

  // Add required resources.
  untrusted_source->AddResourcePath("favicon.ico", IDR_MILLIX_APP_FAVICON_HTML); 
  untrusted_source->AddResourcePath("withdrawal.mp3", IDR_MILLIX_APP_WITHDRAWAL_MP3);
  untrusted_source->AddResourcePath("static/css/2.chunk.css", IDR_MILLIX_APP_CHUNK_0_CSS);
  untrusted_source->AddResourcePath("static/css/2.chunk.css.map", IDR_MILLIX_APP_CHUNK_0_CSS_MAP);
  untrusted_source->AddResourcePath("static/css/main.chunk.css", IDR_MILLIX_APP_MAIN_CSS);
  untrusted_source->AddResourcePath("static/css/main.chunk.css.map", IDR_MILLIX_APP_MAIN_CSS_MAP);
  untrusted_source->AddResourcePath("static/js/2.chunk.js", IDR_MILLIX_APP_CHUNK_0_JS);
  untrusted_source->AddResourcePath("static/js/2.chunk.js.map", IDR_MILLIX_APP_CHUNK_0_JS_MAP);
  untrusted_source->AddResourcePath("static/js/3.chunk.js", IDR_MILLIX_APP_CHUNK_1_JS);
  untrusted_source->AddResourcePath("static/js/3.chunk.js.map", IDR_MILLIX_APP_CHUNK_1_JS_MAP);
  untrusted_source->AddResourcePath("static/js/main.chunk.js", IDR_MILLIX_APP_MAIN_JS);
  untrusted_source->AddResourcePath("static/js/main.chunk.js.map", IDR_MILLIX_APP_MAIN_JS_MAP);
  untrusted_source->AddResourcePath("static/js/runtime-main.js", IDR_MILLIX_APP_RUNTIME_JS);
  untrusted_source->AddResourcePath("static/js/runtime-main.js.map", IDR_MILLIX_APP_RUNTIME_JS_MAP);
  untrusted_source->AddResourcePath("static/media/primeicons.eot", IDR_MILLIX_APP_PRIMEICONS_EOT);
  untrusted_source->AddResourcePath("static/media/primeicons.svg", IDR_MILLIX_APP_PRIMEICONS_SVG);
  untrusted_source->AddResourcePath("static/media/primeicons.woff", IDR_MILLIX_APP_PRIMEICONS_WOFF);
  untrusted_source->AddResourcePath("static/media/primeicons.ttf", IDR_MILLIX_APP_PRIMEICONS_TTF);
  untrusted_source->AddResourcePath("static/media/roboto_bold.woff", IDR_MILLIX_APP_ROBOTO_BOLD_WOFF);
  untrusted_source->AddResourcePath("static/media/roboto_bold.woff", IDR_MILLIX_APP_ROBOTO_BOLD_WOFF2);
  untrusted_source->AddResourcePath("static/media/roboto_medium.woff", IDR_MILLIX_APP_ROBOTO_MEDIUM_WOFF);
  untrusted_source->AddResourcePath("static/media/roboto_medium.woff", IDR_MILLIX_APP_ROBOTO_MEDIUM_WOFF2);
  untrusted_source->AddResourcePath("static/media/roboto_regular.woff", IDR_MILLIX_APP_ROBOTO_REGULAR_WOFF);
  untrusted_source->AddResourcePath("static/media/roboto_regular.woff2", IDR_MILLIX_APP_ROBOTO_REGULAR_WOFF2);
  untrusted_source->AddResourcePath("static/media/import_token.png", IDR_MILLIX_APP_IMPORT_TOKEN_PNG);
  untrusted_source->AddResourcePath("static/media/import_token_result.png", IDR_MILLIX_APP_IMPORT_TOKEN_RESULT_PNG);
  untrusted_source->AddResourcePath("static/media/import_token_confirmation.png", IDR_MILLIX_APP_IMPORT_TOKEN_CONFIRMATION_PNG);
  untrusted_source->AddResourcePath("static/media/wallet.png", IDR_MILLIX_APP_WALLET_PNG);
  untrusted_source->AddResourcePath("static/media/fiatleak.png", IDR_MILLIX_APP_FIATLEAK_PNG);
  untrusted_source->AddResourcePath("static/media/tangled.png", IDR_MILLIX_APP_TANGLED_PNG);

  untrusted_source->SetDefaultResource(IDR_MILLIX_APP_UNTRUSTED_HTML);

  untrusted_source->DisableTrustedTypesCSP();

  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc, "connect-src https: blob:;");

  untrusted_source->OverrideContentSecurityPolicy(
        network::mojom::CSPDirectiveName::DefaultSrc, "default-src 'self' 'unsafe-inline';");

  untrusted_source->OverrideContentSecurityPolicy(
        network::mojom::CSPDirectiveName::ImgSrc, "img-src 'self' data: blob:;");

  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::MediaSrc, "media-src 'self' data: blob:;");

  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ObjectSrc, "object-src 'self' data: blob:;");

  untrusted_source->OverrideContentSecurityPolicy(
     network::mojom::CSPDirectiveName::FrameSrc, "frame-src 'self' data: blob:;");

  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc, "script-src tangled://resources 'self' 'unsafe-inline';");
  untrusted_source->AddFrameAncestor(GURL(chrome::kChromeUIMillixAppURL));

  Profile* profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource::Add(profile, untrusted_source);
}