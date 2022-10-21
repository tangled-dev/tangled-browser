#ifndef CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_
#define CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_
#pragma once

#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "ui/webui/mojo_web_ui_controller.h"

namespace ui {
class NativeTheme;
}

// The WebUI for tangled://millix
class MillixAppUI : public ui::MojoWebUIController {
  class MillixAppMessageHandler : public content::WebUIMessageHandler {
   public:
    MillixAppMessageHandler();
    ~MillixAppMessageHandler() override;

   protected:
    void RegisterMessages() override;

   private:
    void UpdateMillixWallet(const base::Value::List& args);
    void HandleInitializeMessage(const base::Value::List& args);
  };

 public:
  explicit MillixAppUI(content::WebUI* web_ui);
  ~MillixAppUI() override;
};

#endif  // CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_