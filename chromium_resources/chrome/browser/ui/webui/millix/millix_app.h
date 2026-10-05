#ifndef CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_
#define CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_
#pragma once

#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/webui_config.h"
#include "content/public/common/url_constants.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "ui/webui/mojo_web_ui_controller.h"

namespace content {
class WebContents;
}

namespace ui {
class NativeTheme;
}

class MillixAppUI;

// Always enabled: a tangled://millix tab can be restored at startup before the
// millix node is ready. The page waits for the node API config instead (see
// MillixAppMessageHandler::SendNodeApiConfigWhenReady).
class MillixAppUIConfig : public content::DefaultWebUIConfig<MillixAppUI> {
 public:
  MillixAppUIConfig()
      : DefaultWebUIConfig(content::kChromeUIScheme,
                           chrome::kChromeUIMillixAppHost) {}
};

// The WebUI for tangled://millix
class MillixAppUI : public ui::MojoWebUIController {
  class MillixAppMessageHandler : public content::WebUIMessageHandler {
   public:
    MillixAppMessageHandler();
    ~MillixAppMessageHandler() override;

   protected:
    void RegisterMessages() override;
    void OnJavascriptDisallowed() override;

   private:
    void UpdateMillixWallet(const base::ListValue& args);
    void HandleInitializeMessage(const base::ListValue& args);
    // Sends the node API config to the page once the millix bar has read it
    // from node.json, retrying until then.
    void SendNodeApiConfigWhenReady();
    // The tangled://millix-bar contents of the current tabbed browser, if any.
    content::WebContents* GetMillixBarContents();

    base::WeakPtrFactory<MillixAppMessageHandler> weak_factory_{this};
  };

 public:
  explicit MillixAppUI(content::WebUI* web_ui);
  ~MillixAppUI() override;
};

#endif  // CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_