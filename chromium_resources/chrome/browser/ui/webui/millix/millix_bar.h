#ifndef CHROME_BROWSER_UI_WEBUI_MILLIX_BAR_UI_H_
#define CHROME_BROWSER_UI_WEBUI_MILLIX_BAR_UI_H_
#pragma once

#include "base/compiler_specific.h"
#include "base/memory/raw_ptr.h"
#include "base/values.h"
#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/webui_config.h"
#include "content/public/common/url_constants.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "ui/webui/mojo_web_ui_controller.h"

#include <optional>
#include <string>

#include "base/memory/weak_ptr.h"

namespace ui {
  class NativeTheme;
}

class MillixBarUI;

class MillixBarUIConfig : public content::DefaultWebUIConfig<MillixBarUI> {
 public:
  MillixBarUIConfig()
      : DefaultWebUIConfig(content::kChromeUIScheme,
                           chrome::kChromeUIMillixBarHost) {}
};

// The WebUI for tangled://millix-bar
class MillixBarUI : public ui::MojoWebUIController {
 class MillixBarMessageHandler: public content::WebUIMessageHandler {
   public:
    MillixBarMessageHandler();
    ~MillixBarMessageHandler() override;
    std::string node_id;
    std::string node_signature;
   protected:
    void RegisterMessages() override;
   private:
    void HandleInitializeMessage(const base::ListValue& args);
    void ShowMillixWallet(const base::ListValue& args);
    void UpdateMillixWallet(const base::ListValue& args);
    void RestarMillixNode(const base::ListValue& args);
 };

 public:
  explicit MillixBarUI(content::WebUI* web_ui);
  ~MillixBarUI() override;
  std::string GetNodeId() const;
  std::string GetNodeSignature() const;
  void ReadNodeConfig();
 private:
  // Called on the UI thread with the contents of node.json, if it was read.
  void OnNodeConfigRead(std::optional<std::string> json);
  void OnUpdateNodeApiConfig();
  raw_ptr<MillixBarMessageHandler> message_handler;
  // Node config reads and retries are bound to this, so none run after the
  // WebUI is destroyed (e.g. when tangled://millix-bar is reloaded).
  base::WeakPtrFactory<MillixBarUI> weak_factory_{this};
};

#endif  // CHROME_BROWSER_UI_WEBUI_MILLIX_BAR_UI_H_