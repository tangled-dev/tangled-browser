#ifndef CHROME_BROWSER_UI_WEBUI_MILLIX_BAR_UI_H_
#define CHROME_BROWSER_UI_WEBUI_MILLIX_BAR_UI_H_
#pragma once

#include "base/compiler_specific.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "ui/webui/mojo_web_ui_controller.h"
#include "net/base/io_buffer.h"
#include "storage/browser/file_system/file_stream_reader.h"

namespace ui {
  class NativeTheme;
}

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
    void HandleInitializeMessage(const base::Value::List& args);
    void ShowMillixWallet(const base::Value::List& args);
    void UpdateMillixWallet(const base::Value::List& args);
    void RestarMillixNode(const base::Value::List& args);
 };

 public:
  explicit MillixBarUI(content::WebUI* web_ui);
  ~MillixBarUI() override;
  std::string GetNodeId() const;
  std::string GetNodeSignature() const;
  void ReadNodeConfig();
 private: 
  void ReadNodeConfigJson(int64_t size);
  void OnReadNodeConfigJson(int result);
  void OnUpdateNodeApiConfig();
  std::unique_ptr<storage::FileStreamReader> node_config_file_reader;
  scoped_refptr<net::IOBufferWithSize> node_config_buffer;
  MillixBarMessageHandler* message_handler;
};

#endif  // CHROME_BROWSER_UI_WEBUI_MILLIX_BAR_UI_H_