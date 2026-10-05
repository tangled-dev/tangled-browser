#ifndef CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_
#define CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_
#pragma once

#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/webui_config.h"
#include "content/public/common/url_constants.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "base/values.h"
#include "ui/webui/mojo_web_ui_controller.h"

namespace ui {
class NativeTheme;
}

class MillixAppUI;

class MillixAppUIConfig : public content::DefaultWebUIConfig<MillixAppUI> {
 public:
  MillixAppUIConfig()
      : DefaultWebUIConfig(content::kChromeUIScheme,
                           chrome::kChromeUIMillixAppHost) {}

  // The wallet is only available once the millix bar is shown.
  bool IsWebUIEnabled(content::BrowserContext* browser_context) override;
};

// The WebUI for tangled://millix
class MillixAppUI : public ui::MojoWebUIController {
  class MillixAppMessageHandler : public content::WebUIMessageHandler {
   public:
    MillixAppMessageHandler();
    ~MillixAppMessageHandler() override;

   protected:
    void RegisterMessages() override;

   private:
    void UpdateMillixWallet(const base::ListValue& args);
    void HandleInitializeMessage(const base::ListValue& args);
  };

 public:
  explicit MillixAppUI(content::WebUI* web_ui);
  ~MillixAppUI() override;
};

#endif  // CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_