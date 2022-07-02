// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <list>
#include "boost/algorithm/string.hpp"
#include "boost/scoped_ptr.hpp"
#include "core/pdcodegen/pds_option.pb.h"
#include "core/pdcodegen/plugin/strutil.h"
#include "core/pdcodegen/plugin/substitute.h"
#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/compiler/plugin.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"
#include "google/protobuf/io/zero_copy_stream.h"
using std::string;
using google::protobuf::Descriptor;
using google::protobuf::FileDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::MethodDescriptor;
using google::protobuf::ServiceDescriptor;
using google::protobuf::compiler::CodeGenerator;
using google::protobuf::compiler::GeneratorContext;
using google::protobuf::HasSuffixString;
using google::protobuf::StripSuffixString;
namespace google {
namespace protobuf {
string dots_to_colons(const string& name) {
  return StringReplace(name, ".", "::", true);
}
string full_class_name(const Descriptor* descriptor) {
  // Find "outer", the descriptor of the top-level message in which
  // "descriptor" is embedded.
  const Descriptor* outer = descriptor;
  while (outer->containing_type() != NULL) {
    outer = outer->containing_type();
  }
  return outer->full_name();
}
}  // namespace protobuf
}  // namespace google
string strip_proto(const string& filename) {
  if (HasSuffixString(filename, ".protolevel")) {
    return StripSuffixString(filename, ".protolevel");
  } else {
    return StripSuffixString(filename, ".proto");
  }
}
void string_format(std::string& source) {  // NOLINT
  size_t len = source.length();
  std::string sep = "_";
  for (int i = 0; i < len; i++) {
    if (source[i] >= 'A' && source[i] <= 'Z') {
      source[i] += 32;
      if (i == 0) {
        continue;
      }
      source.insert(i, sep);
      i++;
      len++;
    }
  }
}
bool valid_service_method(const std::vector<const MethodDescriptor*>& methods) {
  if (methods.size() != 2) {
    return false;
  }
  if (methods[0]->name() == "inference" && methods[1]->name() == "debug") {
    return true;
  }
  if (methods[1]->name() == "inference" && methods[0]->name() == "debug") {
    return true;
  }
  return false;
}

#ifdef BCLOUD
class PdsCodeGenerator : public CodeGenerator {
 public:
  virtual bool Generate(const FileDescriptor* file,
                        const string& parameter,
                        GeneratorContext* context,
                        std::string* error) const {
    const string header = strip_proto(file->name()) + ".pb.h";
    const string body = strip_proto(file->name()) + ".pb.cc";
    bool include_inserted = false;
    for (int i = 0; i < file->service_count(); ++i) {
      const ServiceDescriptor* descriptor = file->service(i);
      if (!descriptor) {
        *error = "get descriptor failed";
        return false;
      }
      pds::PaddleServiceOption options =
          descriptor->options().GetExtension(pds::options);
      bool generate_impl = options.generate_impl();
      bool generate_stub = options.generate_stub();
      if (!generate_impl && !generate_stub) {
        return true;
      }
      if (!include_inserted) {
        boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
            context->OpenForInsert(header, "includes"));
        google::protobuf::io::Printer printer(output.get(), '$');
        if (generate_impl) {
          printer.Print("#include \"core/predictor/common/inner_common.h\"\n");
          printer.Print("#include \"core/predictor/framework/service.h\"\n");
          printer.Print("#include \"core/predictor/framework/manager.h\"\n");
          printer.Print(
              "#include \"core/predictor/framework/service_manager.h\"\n");
        }
        if (generate_stub) {
          printer.Print("#include <baidu/rpc/parallel_channel.h>\n");
          printer.Print("#include \"core/sdk-cpp/include/factory.h\"\n");
          printer.Print("#include \"core/sdk-cpp/include/stub.h\"\n");
          printer.Print("#include \"core/sdk-cpp/include/stub_impl.h\"\n");
        }
        include_inserted = true;
      }
      const std::string& class_name = descriptor->name();
      const std::string& service_name = descriptor->name();
      // xxx.ph.h
      {
        if (generate_impl) {
          // service scope
          // namespace scope
          boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
              context->OpenForInsert(header, "namespace_scope"));
          google::protobuf::io::Printer printer(output.get(), '$');
          if (!generate_paddle_serving_head(
                  &printer, descriptor, error, service_name, class_name)) {
            return false;
          }
        }
        if (generate_stub) {
          // service class scope

          // namespace scope
          {
            boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream>
                output(context->OpenForInsert(header, "namespace_scope"));
            google::protobuf::io::Printer printer(output.get(), '$');
            if (!generate_paddle_serving_stub_head(
                    &printer, descriptor, error, service_name, class_name)) {
              return false;
            }
          }
        }
      }
      // xxx.pb.cc
      {
        if (generate_impl) {
          // service scope
          // namespace scope
          boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
              context->OpenForInsert(body, "namespace_scope"));
          google::protobuf::io::Printer printer(output.get(), '$');
          if (!generate_paddle_serving_body(
                  &printer, descriptor, error, service_name, class_name)) {
            return false;
          }
        }
        if (generate_stub) {
          // service class scope
          {}  // namespace scope
          {
            boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream>
                output(context->OpenForInsert(body, "namespace_scope"));
            google::protobuf::io::Printer printer(output.get(), '$');
            if (!generate_paddle_serving_stub_body(
                    &printer, descriptor, error, service_name, class_name)) {
              return false;
            }
          }
        }
      }
    }
    return true;
  }

 private:
  bool generate_paddle_serving_head(google::protobuf::io::Printer* printer,
                                    const ServiceDescriptor* descriptor,
                                    string* error,
                                    const std::string& service_name,
                                    const std::string& class_name) const {
    std::vector<const MethodDescriptor*> methods;
    for (int i = 0; i < descriptor->method_count(); ++i) {
      methods.push_back(descriptor->method(i));
    }
    if (!valid_service_method(methods)) {
      *error = "Service can only contains two methods: inferend, debug";
      return false;
    }
    std::string variable_name = class_name;
    string_format(variable_name);
    printer->Print(
        "class $name$Impl : public $name$ {\n"
        "public:\n"
        "  virtual ~$name$Impl() {}\n"
        "  static $name$Impl& instance() {\n"
        "    return _s_$variable_name$_impl;\n"
        "  }\n\n"
        "  $name$Impl(const std::string& service_name) {\n"
        "    REGIST_FORMAT_SERVICE(\n"
        "            service_name, &$name$Impl::instance());\n"
        "  }\n\n",
        "name",
        class_name,
        "variable_name",
        variable_name);
    for (int i = 0; i < methods.size(); i++) {
      const MethodDescriptor* m = methods[i];
      printer->Print(
          "  virtual void $name$(google::protobuf::RpcController* cntl_base,\n"
          "          const $input_name$* request,\n"
          "          $output_name$* response,\n"
          "          google::protobuf::Closure* done);\n\n",
          "name",
          m->name(),
          "input_name",
          google::protobuf::dots_to_colons(m->input_type()->full_name()),
          "output_name",
          google::protobuf::dots_to_colons(m->output_type()->full_name()));
    }
    printer->Print(
        "  static $name$Impl _s_$variable_name$_impl;\n"
        "};",
        "name",
        class_name,
        "variable_name",
        variable_name);
    return true;
  }
  bool generate_paddle_serving_body(google::protobuf::io::Printer* printer,
                                    const ServiceDescriptor* descriptor,
                                    string* error,
                                    const std::string& service_name,
                                    const std::string& class_name) const {
    std::vector<const MethodDescriptor*> methods;
    for (int i = 0; i < descriptor->method_count(); ++i) {
      methods.push_back(descriptor->method(i));
    }
    if (!valid_service_method(methods)) {
      *error = "Service can only contains two methods: inferend, debug";
      return false;
    }
    std::string variable_name = class_name;
    string_format(variable_name);
    for (int i = 0; i < methods.size(); i++) {
      const MethodDescriptor* m = methods[i];
      printer->Print("void $name$Impl::$method$(\n",
                     "name",
                     class_name,
                     "method",
                     m->name());
      printer->Print(
          "        google::protobuf::RpcController* cntl_base,\n"
          "        const $input_name$* request,\n"
          "        $output_name$* response,\n"
          "        google::protobuf::Closure* done) {\n"
          "   struct timeval tv;\n"
          "   gettimeofday(&tv, NULL);"
          "   long start = tv.tv_sec * 1000000 + tv.tv_usec;",
          "input_name",
          google::protobuf::dots_to_colons(m->input_type()->full_name()),
          "output_name",
          google::protobuf::dots_to_colons(m->output_type()->full_name()));
      if (m->name() == "inference") {
        std::string inference_body = "";
        inference_body += "  brpc::ClosureGuard done_guard(done);\n";
        inference_body += "  brpc::Controller* cntl = \n";
        inference_body += "        static_cast<brpc::Controller*>(cntl_base);\n";
        inference_body += "  cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);\n";
        inference_body += "  uint64_t log_id = request->log_id();\n";
        inference_body += "  cntl->set_log_id(log_id);\n";
        inference_body += "  ::baidu::paddle_serving::predictor::InferService* svr = \n";
        inference_body += "       ";
        inference_body += "::baidu::paddle_serving::predictor::InferServiceManager::instance(";
        inference_body += ").item(\"$service$\");\n";
        inference_body += "  if (svr == NULL) {\n";
        inference_body += "    LOG(ERROR) << \"(logid=\" << log_id << \") Not found service: ";
        inference_body += "$service$\";\n";
        inference_body += "    cntl->SetFailed(404, \"Not found service: $service$\");\n";
        inference_body += "    return ;\n";
        inference_body += "  }\n";
        inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") ";
        inference_body += "remote_side=\[\" << cntl->remote_side() << ";  // NOLINT
        inference_body += "\"\]\";\n";
        inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") ";
        inference_body += "local_side=\[\" << cntl->local_side() << ";  // NOLINT
        inference_body += "\"\]\";\n";
        inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") ";
        inference_body += "service_name=\[\" << \"$name$\" << \"\]\";\n";  // NOLINT
        if (service_name == "GeneralModelService") {
          inference_body += "uint64_t key = 0;";
          inference_body += "int err_code = 0;";
          inference_body += "if (RequestCache::GetSingleton()->Get(*request, response, &key) != 0) {";
          inference_body += "  err_code = svr->inference(request, response, log_id);";
          inference_body += "  if (err_code != 0) {";
          inference_body += "    LOG(WARNING)";
          inference_body += "        << \"(logid=\" << log_id << \") Failed call inferservice[GeneralModelService], name[GeneralModelService]\"";
          inference_body += "        << \", error_code: \" << err_code;";
          inference_body += "    cntl->SetFailed(err_code, \"InferService inference failed!\");";
          inference_body += "  } else {";
          inference_body += "    RequestCache::GetSingleton()->Put(*request, *response, &key);";
          inference_body += "  }";
          inference_body += "} else {";
          inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") Get from cache\";";
          inference_body += "}";
        } else {
          inference_body += "  int err_code = svr->inference(request, response, log_id);\n";
          inference_body += "  if (err_code != 0) {\n";
          inference_body += "    LOG(WARNING)\n";
          inference_body += "        << \"(logid=\" << log_id << \") Failed call ";
          inference_body += "inferservice[$name$], name[$service$]\"\n";
          inference_body += "        << \", error_code: \" << err_code;\n";
          inference_body += "    cntl->SetFailed(err_code, \"InferService inference ";
          inference_body += "failed!\");\n";
          inference_body += "  }\n";
        }
        inference_body += "  gettimeofday(&tv, NULL);\n";
        inference_body += "  long end = tv.tv_sec * 1000000 + tv.tv_usec;\n";
        if (service_name == "GeneralModelService") {
          inference_body += "  std::ostringstream oss;\n";
          inference_body += "  oss << \"[serving]\"\n";
          inference_body += "      << \"logid=\" << log_id << \",\";\n";
          inference_body += "  int op_num = response->profile_time_size() / 2;\n";
          inference_body += "  for (int i = 0; i < op_num; ++i) {\n";
          inference_body += "    double t = (response->profile_time(i * 2 + 1)\n";
          inference_body += "                - response->profile_time(i * 2)) / 1000.0;\n";
          inference_body += "    oss << \"op\" << i << \"=\" << t << \"ms,\";\n";
          inference_body += "  }\n";
          inference_body += "  double total_time = (end - start) / 1000.0;\n";
          inference_body += "  oss << \"cost=\" << total_time << \"ms.\";\n";
          inference_body += "  // flush notice log\n";
          inference_body += "  LOG(INFO) << oss.str();\n";
          inference_body += "  response->add_profile_time(start);\n";
          inference_body += "  response->add_profile_time(end);\n";
          inference_body += "  if (::baidu::paddle_serving::predictor::PrometheusMetric::Enabled()) {\n";
          inference_body += "  if (err_code == 0) {\n";
          inference_body += "    ::baidu::paddle_serving::predictor::PrometheusMetricManager::\n";
          inference_body += "        GetGeneralSingleton()\n";
          inference_body += "            ->MetricQuerySuccess()\n";
          inference_body += "            .Increment(1);\n";
          inference_body += "  } else {\n";
          inference_body += "    ::baidu::paddle_serving::predictor::PrometheusMetricManager::\n";
          inference_body += "        GetGeneralSingleton()\n";
          inference_body += "            ->MetricQueryFailure()\n";
          inference_body += "            .Increment(1);\n";
          inference_body += "  }\n";
          inference_body += "  ::baidu::paddle_serving::predictor::PrometheusMetricManager::\n";
          inference_body += "      GetGeneralSingleton()\n";
          inference_body += "          ->MetricQueryDuration()\n";
          inference_body += "          .Increment(total_time * 1000);\n";
          inference_body += "  }\n";
        } else {
          inference_body += "  // flush notice log\n";
          inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") tc=\[\" << (end - ";  // NOLINT
          inference_body += "start) << \"\]\";\n";
        }
        printer->Print(
            inference_body.c_str(),
            "name",
            class_name,
            "service",
            service_name);
      }
      if (m->name() == "debug") {
        printer->Print(
            "  baidu::rpc::ClosureGuard done_guard(done);\n"
            "  baidu::rpc::Controller* cntl = \n"
            "        static_cast<baidu::rpc::Controller*>(cntl_base);\n"
            "  cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);\n"
            "  uint64_t log_id = equest->log_id();\n"
            "  cntl->set_log_id(log_id);\n"
            "  ::baidu::paddle_serving::predictor::InferService* svr = \n"
            "       "
            "::baidu::paddle_serving::predictor::InferServiceManager::instance("
            ").item(\"$service$\");\n"
            "  if (svr == NULL) {\n"
            "    LOG(ERROR) << \"(logid=\" << log_id << \") Not found service: "
            "$service$\";\n"
            "    cntl->SetFailed(404, \"Not found service: $service$\");\n"
            "    return ;\n"
            "  }\n"
            "  LOG(INFO) << \"(logid=\" << log_id << \") remote_side=\[\" "  // NOLINT
            "<< cntl->remote_side() << \"\]\";\n"
            "  LOG(INFO) << \"(logid=\" << log_id << \") local_side=\[\" "  // NOLINT
            "<< cntl->local_side() << \"\]\";\n"
            "  LOG(INFO) << \"(logid=\" << log_id << \") service_name=\[\" "  // NOLINT
            "<< \"$name$\" << \"\]\";\n"
            "  butil::IOBufBuilder debug_os;\n"
            "  int err_code = svr->inference(request, response, log_id, "
            "&debug_os);\n"
            "  if (err_code != 0) {\n"
            "    LOG(WARNING)\n"
            "        << \"(logid=\" << log_id << \") Failed call "
            "inferservice[$name$], name[$service$]\"\n"
            "        << \", error_code: \" << err_code;\n"
            "    cntl->SetFailed(err_code, \"InferService inference "
            "failed!\");\n"
            "  }\n"
            "  debug_os.move_to(cntl->response_attachment());\n"
            "  gettimeofday(&tv, NULL);\n"
            "  long end = tv.tv_sec * 1000000 + tv.tv_usec;\n"
            "  // flush notice log\n"
            "  LOG(INFO) << \"(logid=\" << log_id << \") tc=\[\" << (end - "  // NOLINT
            "start) << \"\]\";\n"
            "  LOG(INFO)\n"
            "      << \"(logid=\" << log_id << \") TC=[\" << (end - start) << "
            "\"] Received debug "
            "request[log_id=\" << cntl->log_id()\n"
            "      << \"] from \" << cntl->remote_side()\n"
            "      << \" to \" << cntl->local_side();\n",
            "name",
            class_name,
            "service",
            service_name);
      }
      printer->Print("}\n");
    }
    printer->Print(
        "$name$Impl $name$Impl::_s_$variable_name$_impl(\"$service$\");\n",
        "name",
        class_name,
        "variable_name",
        variable_name,
        "service",
        service_name);
    return true;
  }
  bool generate_paddle_serving_stub_head(google::protobuf::io::Printer* printer,
                                         const ServiceDescriptor* descriptor,
                                         string* error,
                                         const std::string& service_name,
                                         const std::string& class_name) const {
    printer->Print(
        "class $name$_StubCallMapper : public baidu::rpc::CallMapper {\n"
        "private:\n"
        "   uint32_t _package_size;\n"
        "   baidu::paddle_serving::sdk_cpp::Stub* _stub_handler;\n"
        "public:\n",
        "name",
        class_name);
    printer->Indent();
    printer->Print(
        "$name$_StubCallMapper(uint32_t package_size, "
        "baidu::paddle_serving::sdk_cpp::Stub* stub) {\n"
        "   _package_size = package_size;\n"
        "   _stub_handler = stub;\n"
        "}\n",
        "name",
        class_name);

    printer->Print(
        "baidu::rpc::SubCall default_map(\n"
        "        int channel_index,\n"
        "        const google::protobuf::MethodDescriptor* method,\n"
        "        const google::protobuf::Message* request,\n"
        "        google::protobuf::Message* response) {\n"
        "   baidu::paddle_serving::sdk_cpp::TracePackScope "
        "scope(\"default_map\", channel_index);",
        "name",
        class_name);
    printer->Indent();

    if (!generate_paddle_serving_stub_default_map(
            printer, descriptor, error, service_name, class_name)) {
      return false;
    }

    printer->Outdent();
    printer->Print("}\n");

    printer->Print(
        "baidu::rpc::SubCall sub_package_map(\n"
        "        int channel_index,\n"
        "        const google::protobuf::MethodDescriptor* method,\n"
        "        const google::protobuf::Message* request,\n"
        "        google::protobuf::Message* response) {\n"
        "   baidu::paddle_serving::sdk_cpp::TracePackScope scope(\"sub_map\", "
        "channel_index);",
        "name",
        class_name);
    printer->Indent();

    std::vector<const FieldDescriptor*> in_shared_fields;
    std::vector<const FieldDescriptor*> in_item_fields;
    const MethodDescriptor* md = descriptor->FindMethodByName("inference");
    if (!md) {
      *error = "not found inference method!";
      return false;
    }
    for (int i = 0; i < md->input_type()->field_count(); ++i) {
      const FieldDescriptor* fd = md->input_type()->field(i);
      if (!fd) {
        *error = "invalid fd at: " + i;
        return false;
      }
      bool pack_on = fd->options().GetExtension(pds::pack_on);
      if (pack_on && !fd->is_repeated()) {
        *error = "Pack fields must be repeated, field: " + fd->name();
        return false;
      }
      if (pack_on) {
        in_item_fields.push_back(fd);
      } else {
        in_shared_fields.push_back(fd);
      }
    }

    if (!generate_paddle_serving_stub_package_map(printer,
                                                  descriptor,
                                                  error,
                                                  service_name,
                                                  class_name,
                                                  in_shared_fields,
                                                  in_item_fields)) {
      return false;
    }
    printer->Outdent();
    printer->Print("}\n");

    printer->Print(
        "baidu::rpc::SubCall Map(\n"
        "        int channel_index,\n"
        "        const google::protobuf::MethodDescriptor* method,\n"
        "        const google::protobuf::Message* request,\n"
        "        google::protobuf::Message* response) {\n",
        "name",
        class_name);
    printer->Indent();

    if (in_item_fields.size() <= 0) {
      printer->Print(
          "// No packed items found in proto file, use default map method\n"
          "return default_map(channel_index, method, request, response);\n");
    } else {
      printer->Print(
          "butil::Timer tt(butil::Timer::STARTED);\n"
          "baidu::rpc::SubCall ret;\n"
          "if (_package_size == 0) {\n"
          "   ret = default_map(channel_index, method, request, response);\n"
          "} else {\n"
          "   ret = sub_package_map(channel_index, method, request, "
          "response);\n"
          "}\n"
          "tt.stop();\n"
          "if (ret.flags != baidu::rpc::SKIP_SUB_CHANNEL && ret.method != "
          "NULL) {\n"
          "   _stub_handler->update_latency(tt.u_elapsed(), \"pack_map\");\n"
          "}\n"
          "return ret;\n");
    }

    printer->Outdent();
    printer->Print("}\n");

    printer->Outdent();
    printer->Print("};\n");

    ////////////////////////////////////////////////////////////////
    printer->Print(
        "class $name$_StubResponseMerger : public baidu::rpc::ResponseMerger "
        "{\n"
        "private:\n"
        "   uint32_t _package_size;\n"
        "   baidu::paddle_serving::sdk_cpp::Stub* _stub_handler;\n"
        "public:\n",
        "name",
        class_name);
    printer->Indent();
    printer->Print(
        "$name$_StubResponseMerger(uint32_t package_size, "
        "baidu::paddle_serving::sdk_cpp::Stub* stub) {\n"
        "   _package_size = package_size;\n"
        "   _stub_handler = stub;\n"
        "}\n",
        "name",
        class_name);

    printer->Print(
        "baidu::rpc::ResponseMerger::Result default_merge(\n"
        "        google::protobuf::Message* response,\n"
        "        const google::protobuf::Message* sub_response) {\n"
        "   baidu::paddle_serving::sdk_cpp::TracePackScope "
        "scope(\"default_merge\");",
        "name",
        class_name);
    printer->Indent();
    if (!generate_paddle_serving_stub_default_merger(
            printer, descriptor, error, service_name, class_name)) {
      return false;
    }
    printer->Outdent();
    printer->Print("}\n");

    printer->Print(
        "baidu::rpc::ResponseMerger::Result sub_package_merge(\n"
        "        google::protobuf::Message* response,\n"
        "        const google::protobuf::Message* sub_response) {\n"
        "   baidu::paddle_serving::sdk_cpp::TracePackScope "
        "scope(\"sub_merge\");",
        "name",
        class_name);
    printer->Indent();
    if (!generate_paddle_serving_stub_package_merger(
            printer, descriptor, error, service_name, class_name)) {
      return false;
    }
    printer->Outdent();
    printer->Print("}\n");

    printer->Print(
        "baidu::rpc::ResponseMerger::Result Merge(\n"
        "        google::protobuf::Message* response,\n"
        "        const google::protobuf::Message* sub_response) {\n",
        "name",
        class_name);
    printer->Indent();
    printer->Print(
        "butil::Timer tt(butil::Timer::STARTED);\n"
        "baidu::rpc::ResponseMerger::Result ret;"
        "if (_package_size <= 0) {\n"
        "    ret = default_merge(response, sub_response);\n"
        "} else {\n"
        "    ret = sub_package_merge(response, sub_response);\n"
        "}\n"
        "tt.stop();\n"
        "if (ret != baidu::rpc::ResponseMerger::FAIL) {\n"
        "   _stub_handler->update_latency(tt.u_elapsed(), \"pack_merge\");\n"
        "}\n"
        "return ret;\n");
    printer->Outdent();
    printer->Print("}\n");

    printer->Outdent();
    printer->Print("};\n");
    return true;
  }
  bool generate_paddle_serving_stub_default_map(
      google::protobuf::io::Printer* printer,
      const ServiceDescriptor* descriptor,
      string* error,
      const std::string& service_name,
      const std::string& class_name) const {
    printer->Print(
        "if (channel_index > 0) { \n"
        "   return baidu::rpc::SubCall::Skip();\n"
        "}\n");
    printer->Print(
        "google::protobuf::Message* cur_res = "
        "_stub_handler->fetch_response();\n"
        "if (cur_res == NULL) {\n"
        "   LOG(INFO) << \"Failed fetch response from stub handler, new it\";\n"
        "   cur_res = response->New();\n"
        "   if (cur_res == NULL) {\n"
        "       LOG(ERROR) << \"Failed new response item!\";\n"
        "       _stub_handler->update_average(1, \"pack_fail\");\n"
        "       return baidu::rpc::SubCall::Bad();\n"
        "   }\n"
        "   return baidu::rpc::SubCall(method, request, cur_res, "
        "baidu::rpc::DELETE_RESPONSE);\n"
        "}\n");
    "LOG(INFO) \n"
    "   << \"[default] Succ map, channel_index: \" << channel_index;\n";
    printer->Print(
        "return baidu::rpc::SubCall(method, request, cur_res, 0);\n");
    return true;
  }
  bool generate_paddle_serving_stub_default_merger(
      google::protobuf::io::Printer* printer,
      const ServiceDescriptor* descriptor,
      string* error,
      const std::string& service_name,
      const std::string& class_name) const {
    printer->Print(
        "try {\n"
        "   response->MergeFrom(*sub_response);\n"
        "   return baidu::rpc::ResponseMerger::MERGED;\n"
        "} catch (const std::exception& e) {\n"
        "   LOG(ERROR) << \"Merge failed.\";\n"
        "   _stub_handler->update_average(1, \"pack_fail\");\n"
        "   return baidu::rpc::ResponseMerger::FAIL;\n"
        "}\n");
    return true;
  }
  bool generate_paddle_serving_stub_package_map(
      google::protobuf::io::Printer* printer,
      const ServiceDescriptor* descriptor,
      string* error,
      const std::string& service_name,
      const std::string& class_name,
      std::vector<const FieldDescriptor*>& in_shared_fields,        // NOLINT
      std::vector<const FieldDescriptor*>& in_item_fields) const {  // NOLINT
    const MethodDescriptor* md = descriptor->FindMethodByName("inference");
    if (!md) {
      *error = "not found inference method!";
      return false;
    }

    printer->Print(
        "const $req_type$* req \n"
        "       = dynamic_cast<const $req_type$*>(request);\n"
        "$req_type$* sub_req = NULL;",
        "req_type",
        google::protobuf::dots_to_colons(md->input_type()->full_name()));

    // 1. pack fields 逐字段计算index范围，并从req copy值sub_req
    printer->Print("\n// 1. 样本字段(必须为repeated类型)按指定下标复制\n");
    for (uint32_t ii = 0; ii < in_item_fields.size(); ii++) {
      const FieldDescriptor* fd = in_item_fields[ii];
      std::string field_name = fd->name();
      printer->Print("\n/////$field_name$\n", "field_name", field_name);
      if (ii == 0) {
        printer->Print(
            "uint32_t total_size = req->$field_name$_size();\n"
            "if (channel_index == 0) {\n"
            "   _stub_handler->update_average(total_size, \"item_size\");\n"
            "}\n",
            "field_name",
            field_name);

        printer->Print(
            "int start = _package_size * channel_index;\n"
            "if (start >= total_size) {\n"
            "   return baidu::rpc::SubCall::Skip();\n"
            "}\n"
            "int end = _package_size * (channel_index + 1);\n"
            "if (end > total_size) {\n"
            "   end = total_size;\n"
            "}\n");

        printer->Print(
            "sub_req = "
            "dynamic_cast<$req_type$*>(_stub_handler->fetch_request());\n"
            "if (sub_req == NULL) {\n"
            "    LOG(ERROR) << \"failed fetch sub_req from stub.\";\n"
            "    _stub_handler->update_average(1, \"pack_fail\");\n"
            "    return baidu::rpc::SubCall::Bad();\n"
            "}\n",
            "name",
            class_name,
            "req_type",
            google::protobuf::dots_to_colons(md->input_type()->full_name()));

      } else {
        printer->Print(
            "if (req->$field_name$_size() != total_size) {\n"
            "    LOG(ERROR) << \"pack field size not consistency: \"\n"
            "               << total_size << \"!=\" << "
            "req->$field_name$_size()\n"
            "               << \", field: $field_name$.\";\n"
            "    _stub_handler->update_average(1, \"pack_fail\");\n"
            "    return baidu::rpc::SubCall::Bad();\n"
            "}\n",
            "field_name",
            field_name);
      }

      printer->Print("for (uint32_t i = start; i < end; ++i) {\n");
      printer->Indent();
      if (fd->cpp_type() ==
          google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
        printer->Print(
            "sub_req->add_$field_name$()->CopyFrom(req->$field_name$(i));\n",
            "field_name",
            field_name);
      } else {
        printer->Print("sub_req->add_$field_name$(req->$field_name$(i));\n",
                       "field_name",
                       field_name);
      }
      printer->Outdent();
      printer->Print("}\n");
    }

    // 2. shared fields逐字段从req copy至sub_req
    printer->Print("\n// 2. 共享字段，从req逐个复制到sub_req\n");
    if (in_item_fields.size() == 0) {
      printer->Print(
          "if (sub_req == NULL) { // no packed items\n"
          "   sub_req = "
          "dynamic_cast<$req_type$*>(_stub_handler->fetch_request());\n"
          "   if (!sub_req) {\n"
          "       LOG(ERROR) << \"failed fetch sub_req from stub handler.\";\n"
          "       _stub_handler->update_average(1, \"pack_fail\");\n"
          "       return baidu::rpc::SubCall::Bad();\n"
          "   }\n"
          "}\n",
          "req_type",
          google::protobuf::dots_to_colons(md->input_type()->full_name()));
    }
    for (uint32_t si = 0; si < in_shared_fields.size(); si++) {
      const FieldDescriptor* fd = in_shared_fields[si];
      std::string field_name = fd->name();
      printer->Print("\n/////$field_name$\n", "field_name", field_name);
      if (fd->is_optional()) {
        printer->Print(
            "if (req->has_$field_name$()) {\n", "field_name", field_name);
        printer->Indent();
      }
      if (fd->cpp_type() ==
              google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE ||
          fd->is_repeated()) {
        printer->Print(
            "sub_req->mutable_$field_name$()->CopyFrom(req->$field_name$());\n",
            "field_name",
            field_name);
      } else {
        printer->Print("sub_req->set_$field_name$(req->$field_name$());\n",
                       "field_name",
                       field_name);
      }
      if (fd->is_optional()) {
        printer->Outdent();
        printer->Print("}\n");
      }
    }

    printer->Print(
        "LOG(INFO)\n"
        "   << \"[pack] Succ map req at: \"\n"
        "   << channel_index;\n");
    printer->Print(
        "google::protobuf::Message* sub_res = "
        "_stub_handler->fetch_response();\n"
        "if (sub_res == NULL) {\n"
        "    LOG(ERROR) << \"failed create sub_res from res.\";\n"
        "    _stub_handler->update_average(1, \"pack_fail\");\n"
        "    return baidu::rpc::SubCall::Bad();\n"
        "}\n"
        "return baidu::rpc::SubCall(method, sub_req, sub_res, 0);\n");
    return true;
  }
  bool generate_paddle_serving_stub_package_merger(
      google::protobuf::io::Printer* printer,
      const ServiceDescriptor* descriptor,
      string* error,
      const std::string& service_name,
      const std::string& class_name) const {
    return generate_paddle_serving_stub_default_merger(
        printer, descriptor, error, service_name, class_name);
  }
  bool generate_paddle_serving_stub_body(google::protobuf::io::Printer* printer,
                                         const ServiceDescriptor* descriptor,
                                         string* error,
                                         const std::string& service_name,
                                         const std::string& class_name) const {
    std::vector<const MethodDescriptor*> methods;
    for (int i = 0; i < descriptor->method_count(); ++i) {
      methods.push_back(descriptor->method(i));
    }
    if (!valid_service_method(methods)) {
      *error = "Service can only contains two methods: inferend, debug";
      return false;
    }

    const MethodDescriptor* md = methods[0];
    std::map<string, string> variables;
    variables["name"] = class_name;
    variables["req_type"] =
        google::protobuf::dots_to_colons(md->input_type()->full_name());
    variables["res_type"] =
        google::protobuf::dots_to_colons(md->output_type()->full_name());
    variables["fullname"] = descriptor->full_name();
    printer->Print(variables,
                   "REGIST_STUB_OBJECT_WITH_TAG(\n"
                   "       $name$_Stub,\n"
                   "       $name$_StubCallMapper,\n"
                   "       $name$_StubResponseMerger,\n"
                   "       $req_type$,\n"
                   "       $res_type$,\n"
                   "       \"$fullname$\");\n");
    variables.clear();
    return true;
  }
};
#else   // #ifdef BCLOUD
class PdsCodeGenerator : public CodeGenerator {
 public:
  virtual bool Generate(const FileDescriptor* file,
                        const string& parameter,
                        GeneratorContext* context,
                        std::string* error) const {
    const string header = strip_proto(file->name()) + ".pb.h";
    const string body = strip_proto(file->name()) + ".pb.cc";
    bool include_inserted = false;
    for (int i = 0; i < file->service_count(); ++i) {
      const ServiceDescriptor* descriptor = file->service(i);
      if (!descriptor) {
        *error = "get descriptor failed";
        return false;
      }
      pds::PaddleServiceOption options =
          descriptor->options().GetExtension(pds::options);
      bool generate_impl = options.generate_impl();
      bool generate_stub = options.generate_stub();
      if (!generate_impl && !generate_stub) {
        return true;
      }
      if (!include_inserted) {
        boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
            context->OpenForInsert(header, "includes"));
        google::protobuf::io::Printer printer(output.get(), '$');
        if (generate_impl) {
          printer.Print("#include \"core/predictor/common/inner_common.h\"\n");
          printer.Print("#include \"core/predictor/framework/service.h\"\n");
          printer.Print("#include \"core/predictor/framework/manager.h\"\n");
          printer.Print(
              "#include \"core/predictor/framework/service_manager.h\"\n");
        }
        if (generate_stub) {
          printer.Print("#include <brpc/parallel_channel.h>\n");
          printer.Print("#include \"core/sdk-cpp/include/factory.h\"\n");
          printer.Print("#include \"core/sdk-cpp/include/stub.h\"\n");
          printer.Print("#include \"core/sdk-cpp/include/stub_impl.h\"\n");
        }
        include_inserted = true;
      }
      const std::string& class_name = descriptor->name();
      const std::string& service_name = descriptor->name();
      // xxx.ph.h
      {
        if (generate_impl) {
          // service scope
          // namespace scope
          boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
              context->OpenForInsert(header, "namespace_scope"));
          google::protobuf::io::Printer printer(output.get(), '$');
          if (!generate_paddle_serving_head(
                  &printer, descriptor, error, service_name, class_name)) {
            return false;
          }
        }
        if (generate_stub) {
          // service class scope

          // namespace scope
          {
            boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream>
                output(context->OpenForInsert(header, "namespace_scope"));
            google::protobuf::io::Printer printer(output.get(), '$');
            if (!generate_paddle_serving_stub_head(
                    &printer, descriptor, error, service_name, class_name)) {
              return false;
            }
          }
        }
      }
      // xxx.pb.cc
      {
        if (generate_impl) {
          // service scope
          // namespace scope
          boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
              context->OpenForInsert(body, "namespace_scope"));
          google::protobuf::io::Printer printer(output.get(), '$');
          if (!generate_paddle_serving_body(
                  &printer, descriptor, error, service_name, class_name)) {
            return false;
          }
        }
        if (generate_stub) {
          // service class scope
          {}  // namespace scope
          {
            boost::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream>
                output(context->OpenForInsert(body, "namespace_scope"));
            google::protobuf::io::Printer printer(output.get(), '$');
            if (!generate_paddle_serving_stub_body(
                    &printer, descriptor, error, service_name, class_name)) {
              return false;
            }
          }
        }
      }
    }
    return true;
  }

 private:
  bool generate_paddle_serving_head(google::protobuf::io::Printer* printer,
                                    const ServiceDescriptor* descriptor,
                                    string* error,
                                    const std::string& service_name,
                                    const std::string& class_name) const {
    std::vector<const MethodDescriptor*> methods;
    for (int i = 0; i < descriptor->method_count(); ++i) {
      methods.push_back(descriptor->method(i));
    }
    if (!valid_service_method(methods)) {
      *error = "Service can only contains two methods: inferend, debug";
      return false;
    }
    std::string variable_name = class_name;
    string_format(variable_name);
    printer->Print(
        "class $name$Impl : public $name$ {\n"
        "public:\n"
        "  virtual ~$name$Impl() {}\n"
        "  static $name$Impl& instance() {\n"
        "    return _s_$variable_name$_impl;\n"
        "  }\n\n"
        "  $name$Impl(const std::string& service_name) {\n"
        "    REGIST_FORMAT_SERVICE(\n"
        "            service_name, &$name$Impl::instance());\n"
        "  }\n\n",
        "name",
        class_name,
        "variable_name",
        variable_name);
    for (int i = 0; i < methods.size(); i++) {
      const MethodDescriptor* m = methods[i];
      printer->Print(
          "  virtual void $name$(google::protobuf::RpcController* cntl_base,\n"
          "          const $input_name$* request,\n"
          "          $output_name$* response,\n"
          "          google::protobuf::Closure* done);\n\n",
          "name",
          m->name(),
          "input_name",
          google::protobuf::dots_to_colons(m->input_type()->full_name()),
          "output_name",
          google::protobuf::dots_to_colons(m->output_type()->full_name()));
    }
    printer->Print(
        "  static $name$Impl _s_$variable_name$_impl;\n"
        "};",
        "name",
        class_name,
        "variable_name",
        variable_name);
    return true;
  }
  bool generate_paddle_serving_body(google::protobuf::io::Printer* printer,
                                    const ServiceDescriptor* descriptor,
                                    string* error,
                                    const std::string& service_name,
                                    const std::string& class_name) const {
    std::vector<const MethodDescriptor*> methods;
    for (int i = 0; i < descriptor->method_count(); ++i) {
      methods.push_back(descriptor->method(i));
    }
    if (!valid_service_method(methods)) {
      *error = "Service can only contains two methods: inferend, debug";
      return false;
    }
    std::string variable_name = class_name;
    string_format(variable_name);
    for (int i = 0; i < methods.size(); i++) {
      const MethodDescriptor* m = methods[i];
      printer->Print("void $name$Impl::$method$(\n",
                     "name",
                     class_name,
                     "method",
                     m->name());
      printer->Print(
          "        google::protobuf::RpcController* cntl_base,\n"
          "        const $input_name$* request,\n"
          "        $output_name$* response,\n"
          "        google::protobuf::Closure* done) {\n"
          "   struct timeval tv;\n"
          "   gettimeofday(&tv, NULL);"
          "   long start = tv.tv_sec * 1000000 + tv.tv_usec;",
          "input_name",
          google::protobuf::dots_to_colons(m->input_type()->full_name()),
          "output_name",
          google::protobuf::dots_to_colons(m->output_type()->full_name()));
      if (m->name() == "inference") {
        std::string inference_body = "";
        inference_body += "  brpc::ClosureGuard done_guard(done);\n";
        inference_body += "  brpc::Controller* cntl = \n";
        inference_body += "        static_cast<brpc::Controller*>(cntl_base);\n";
        inference_body += "  cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);\n";
        inference_body += "  uint64_t log_id = request->log_id();\n";
        inference_body += "  cntl->set_log_id(log_id);\n";
        inference_body += "  ::baidu::paddle_serving::predictor::InferService* svr = \n";
        inference_body += "       ";
        inference_body += "::baidu::paddle_serving::predictor::InferServiceManager::instance(";
        inference_body += ").item(\"$service$\");\n";
        inference_body += "  if (svr == NULL) {\n";
        inference_body += "    LOG(ERROR) << \"(logid=\" << log_id << \") Not found service: ";
        inference_body += "$service$\";\n";
        inference_body += "    cntl->SetFailed(404, \"Not found service: $service$\");\n";
        inference_body += "    return ;\n";
        inference_body += "  }\n";
        inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") ";
        inference_body += "remote_side=\[\" << cntl->remote_side() << ";  // NOLINT
        inference_body += "\"\]\";\n";
        inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") ";
        inference_body += "local_side=\[\" << cntl->local_side() << ";  // NOLINT
        inference_body += "\"\]\";\n";
        inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") ";
        inference_body += "service_name=\[\" << \"$name$\" << \"\]\";\n";  // NOLINT
        if (service_name == "GeneralModelService") {
          inference_body += "uint64_t key = 0;";
          inference_body += "int err_code = 0;";
          inference_body += "if (RequestCache::GetSingleton()->Get(*request, response, &key) != 0) {";
          inference_body += "  err_code = svr->inference(request, response, log_id);";
          inference_body += "  if (err_code != 0) {";
          inference_body += "    LOG(WARNING)";
          inference_body += "        << \"(logid=\" << log_id << \") Failed call inferservice[GeneralModelService], name[GeneralModelService]\"";
          inference_body += "        << \", error_code: \" << err_code;";
          inference_body += "    cntl->SetFailed(err_code, \"InferService inference failed!\");";
          inference_body += "  } else {";
          inference_body += "    RequestCache::GetSingleton()->Put(*request, *response, &key);";
          inference_body += "  }";
          inference_body += "} else {";
          inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") Get from cache\";";
          inference_body += "}";
        } else {
          inference_body += "  int err_code = svr->inference(request, response, log_id);\n";
          inference_body += "  if (err_code != 0) {\n";
          inference_body += "    LOG(WARNING)\n";
          inference_body += "        << \"(logid=\" << log_id << \") Failed call ";
          inference_body += "inferservice[$name$], name[$service$]\"\n";
          inference_body += "        << \", error_code: \" << err_code;\n";
          inference_body += "    cntl->SetFailed(err_code, \"InferService inference ";
          inference_body += "failed!\");\n";
          inference_body += "  }\n";
        }
        inference_body += "  gettimeofday(&tv, NULL);\n";
        inference_body += "  long end = tv.tv_sec * 1000000 + tv.tv_usec;\n";
        if (service_name == "GeneralModelService") {
          inference_body += "  std::ostringstream oss;\n";
          inference_body += "  oss << \"[serving]\"\n";
          inference_body += "      << \"logid=\" << log_id << \",\";\n";
          inference_body += "  int op_num = response->profile_time_size() / 2;\n";
          inference_body += "  for (int i = 0; i < op_num; ++i) {\n";
          inference_body += "    double t = (response->profile_time(i * 2 + 1)\n";
          inference_body += "                - response->profile_time(i * 2)) / 1000.0;\n";
          inference_body += "    oss << \"op\" << i << \"=\" << t << \"ms,\";\n";
          inference_body += "  }\n";
          inference_body += "  double total_time = (end - start) / 1000.0;\n";
          inference_body += "  oss << \"cost=\" << total_time << \"ms.\";\n";
          inference_body += "  // flush notice log\n";
          inference_body += "  LOG(INFO) << oss.str();\n";
          inference_body += "  response->add_profile_time(start);\n";
          inference_body += "  response->add_profile_time(end);\n";
          inference_body += "  if (::baidu::paddle_serving::predictor::PrometheusMetric::Enabled()) {\n";
          inference_body += "  if (err_code == 0) {\n";
          inference_body += "    ::baidu::paddle_serving::predictor::PrometheusMetricManager::\n";
          inference_body += "        GetGeneralSingleton()\n";
          inference_body += "            ->MetricQuerySuccess()\n";
          inference_body += "            .Increment(1);\n";
          inference_body += "  } else {\n";
          inference_body += "    ::baidu::paddle_serving::predictor::PrometheusMetricManager::\n";
          inference_body += "        GetGeneralSingleton()\n";
          inference_body += "            ->MetricQueryFailure()\n";
          inference_body += "            .Increment(1);\n";
          inference_body += "  }\n";
          inference_body += "  ::baidu::paddle_serving::predictor::PrometheusMetricManager::\n";
          inference_body += "      GetGeneralSingleton()\n";
          inference_body += "          ->MetricQueryDuration()\n";
          inference_body += "          .Increment(total_time * 1000);\n";
          inference_body += "  }\n";
        } else {
          inference_body += "  // flush notice log\n";
          inference_body += "  LOG(INFO) << \"(logid=\" << log_id << \") tc=\[\" << (end - ";  // NOLINT
          inference_body += "start) << \"\]\";\n";
        }
        printer->Print(
            inference_body.c_str(),
            "name",
            class_name,
            "service",
            service_name);
      }
      if (m->name() == "debug") {
        printer->Print(
            "  brpc::ClosureGuard done_guard(done);\n"
            "  brpc::Controller* cntl = \n"
            "        static_cast<brpc::Controller*>(cntl_base);\n"
            "  cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);\n"
            "  uint64_t log_id = request->log_id();\n"
            "  cntl->set_log_id(log_id);\n"
            "  ::baidu::paddle_serving::predictor::InferService* svr = \n"
            "       "
            "::baidu::paddle_serving::predictor::InferServiceManager::instance("
            ").item(\"$service$\");\n"
            "  if (svr == NULL) {\n"
            "    LOG(ERROR) << \"(logid=\" << log_id << \") Not found service: "
            "$service$\";\n"
            "    cntl->SetFailed(404, \"Not found service: $service$\");\n"
            "    return ;\n"
            "  }\n"
            "  LOG(INFO) << \"(logid=\" << log_id << \") remote_side=\[\" "  // NOLINT
            " << cntl->remote_side() << \"\]\";\n"
            "  LOG(INFO) << \"(logid=\" << log_id << \") local_side=\[\" "  // NOLINT
            "<< cntl->local_side() << \"\]\";\n"
            "  LOG(INFO) << \"(logid=\" << log_id << \") service_name=\[\" "  // NOLINT
            "<< \"$name$\" << \"\]\";\n"
            "  butil::IOBufBuilder debug_os;\n"
            "  int err_code = svr->inference(request, response, log_id, "
            "&debug_os);\n"
            "  if (err_code != 0) {\n"
            "    LOG(WARNING)\n"
            "        << \"(logid=\" << log_id << \") Failed call "
            "inferservice[$name$], name[$service$]\"\n"
            "        << \", error_code: \" << err_code;\n"
            "    cntl->SetFailed(err_code, \"InferService inference "
            "failed!\");\n"
            "  }\n"
            "  debug_os.move_to(cntl->response_attachment());\n"
            "  gettimeofday(&tv, NULL);\n"
            "  long end = tv.tv_sec * 1000000 + tv.tv_usec;\n"
            "  // flush notice log\n"
            "  LOG(INFO) << \"(logid=\" << log_id << \") tc=\[\" << (end - "  // NOLINT
            "start) << \"\]\";\n"  // NOLINT
            "  LOG(INFO)\n"
            "      << \"(logid=\" << log_id << \") TC=[\" << (end - start) << "
            "\"] Received debug "
            "request[log_id=\" << cntl->log_id()\n"
            "      << \"] from \" << cntl->remote_side()\n"
            "      << \" to \" << cntl->local_side();\n",
            "name",
            class_name,
            "service",
            service_name);
      }
      printer->Print("}\n");
    }
    printer->Print(
        "$name$Impl $name$Impl::_s_$variable_name$_impl(\"$service$\");\n",
        "name",
        class_name,
        "variable_name",
        variable_name,
        "service",
        service_name);
    return true;
  }
  bool generate_paddle_serving_stub_head(google::protobuf::io::Printer* printer,
                                         const ServiceDescriptor* descriptor,
                                         string* error,
                                         const std::string& service_name,
                                         const std::string& class_name) const {
    printer->Print(
        "class $name$_StubCallMapper : public brpc::CallMapper {\n"
        "private:\n"
        "   uint32_t _package_size;\n"
        "   baidu::paddle_serving::sdk_cpp::Stub* _stub_handler;\n"
        "public:\n",
        "name",
        class_name);
    printer->Indent();
    printer->Print(
        "$name$_StubCallMapper(uint32_t package_size, "
        "baidu::paddle_serving::sdk_cpp::Stub* stub) {\n"
        "   _package_size = package_size;\n"
        "   _stub_handler = stub;\n"
        "}\n",
        "name",
        class_name);

    printer->Print(
        "brpc::SubCall default_map(\n"
        "        int channel_index,\n"
        "        const google::protobuf::MethodDescriptor* method,\n"
        "        const google::protobuf::Message* request,\n"
        "        google::protobuf::Message* response) {\n"
        "   baidu::paddle_serving::sdk_cpp::TracePackScope "
        "scope(\"default_map\", channel_index);",
        "name",
        class_name);
    printer->Indent();

    if (!generate_paddle_serving_stub_default_map(
            printer, descriptor, error, service_name, class_name)) {
      return false;
    }

    printer->Outdent();
    printer->Print("}\n");

    printer->Print(
        "brpc::SubCall sub_package_map(\n"
        "        int channel_index,\n"
        "        const google::protobuf::MethodDescriptor* method,\n"
        "        const google::protobuf::Message* request,\n"
        "        google::protobuf::Message* response) {\n"
        "   baidu::paddle_serving::sdk_cpp::TracePackScope scope(\"sub_map\", "
        "channel_index);",
        "name",
        class_name);
    printer->Indent();

    std::vector<const FieldDescriptor*> in_shared_fields;
    std::vector<const FieldDescriptor*> in_item_fields;
    const MethodDescriptor* md = descriptor->FindMethodByName("inference");
    if (!md) {
      *error = "not found inference method!";
      return false;
    }
    for (int i = 0; i < md->input_type()->field_count(); ++i) {
      const FieldDescriptor* fd = md->input_type()->field(i);
      if (!fd) {
        *error = "invalid fd at: " + i;
        return false;
      }
      bool pack_on = fd->options().GetExtension(pds::pack_on);
      if (pack_on && !fd->is_repeated()) {
        *error = "Pack fields must be repeated, field: " + fd->name();
        return false;
      }
      if (pack_on) {
        in_item_fields.push_back(fd);
      } else {
        in_shared_fields.push_back(fd);
      }
    }

    if (!generate_paddle_serving_stub_package_map(printer,
                                                  descriptor,
                                                  error,
                                                  service_name,
                                                  class_name,
                                                  in_shared_fields,
                                                  in_item_fields)) {
      return false;
    }
    printer->Outdent();
    printer->Print("}\n");

    printer->Print(
        "brpc::SubCall Map(\n"
        "        int channel_index,\n"
        "        const google::protobuf::MethodDescriptor* method,\n"
        "        const google::protobuf::Message* request,\n"
        "        google::protobuf::Message* response) {\n",
        "name",
        class_name);
    printer->Indent();

    if (in_item_fields.size() <= 0) {
      printer->Print(
          "// No packed items found in proto file, use default map method\n"
          "return default_map(channel_index, method, request, response);\n");
    } else {
      printer->Print(
          "butil::Timer tt(butil::Timer::STARTED);\n"
          "brpc::SubCall ret;\n"
          "if (_package_size == 0) {\n"
          "   ret = default_map(channel_index, method, request, response);\n"
          "} else {\n"
          "   ret = sub_package_map(channel_index, method, request, "
          "response);\n"
          "}\n"
          "tt.stop();\n"
          "if (ret.flags != brpc::SKIP_SUB_CHANNEL && ret.method != NULL) {\n"
          "   _stub_handler->update_latency(tt.u_elapsed(), \"pack_map\");\n"
          "}\n"
          "return ret;\n");
    }

    printer->Outdent();
    printer->Print("}\n");

    printer->Outdent();
    printer->Print("};\n");

    ////////////////////////////////////////////////////////////////
    printer->Print(
        "class $name$_StubResponseMerger : public brpc::ResponseMerger {\n"
        "private:\n"
        "   uint32_t _package_size;\n"
        "   baidu::paddle_serving::sdk_cpp::Stub* _stub_handler;\n"
        "public:\n",
        "name",
        class_name);
    printer->Indent();
    printer->Print(
        "$name$_StubResponseMerger(uint32_t package_size, "
        "baidu::paddle_serving::sdk_cpp::Stub* stub) {\n"
        "   _package_size = package_size;\n"
        "   _stub_handler = stub;\n"
        "}\n",
        "name",
        class_name);

    printer->Print(
        "brpc::ResponseMerger::Result default_merge(\n"
        "        google::protobuf::Message* response,\n"
        "        const google::protobuf::Message* sub_response) {\n"
        "   baidu::paddle_serving::sdk_cpp::TracePackScope "
        "scope(\"default_merge\");",
        "name",
        class_name);
    printer->Indent();
    if (!generate_paddle_serving_stub_default_merger(
            printer, descriptor, error, service_name, class_name)) {
      return false;
    }
    printer->Outdent();
    printer->Print("}\n");

    printer->Print(
        "brpc::ResponseMerger::Result sub_package_merge(\n"
        "        google::protobuf::Message* response,\n"
        "        const google::protobuf::Message* sub_response) {\n"
        "   baidu::paddle_serving::sdk_cpp::TracePackScope "
        "scope(\"sub_merge\");",
        "name",
        class_name);
    printer->Indent();
    if (!generate_paddle_serving_stub_package_merger(
            printer, descriptor, error, service_name, class_name)) {
      return false;
    }
    printer->Outdent();
    printer->Print("}\n");

    printer->Print(
        "brpc::ResponseMerger::Result Merge(\n"
        "        google::protobuf::Message* response,\n"
        "        const google::protobuf::Message* sub_response) {\n",
        "name",
        class_name);
    printer->Indent();
    printer->Print(
        "butil::Timer tt(butil::Timer::STARTED);\n"
        "brpc::ResponseMerger::Result ret;"
        "if (_package_size <= 0) {\n"
        "    ret = default_merge(response, sub_response);\n"
        "} else {\n"
        "    ret = sub_package_merge(response, sub_response);\n"
        "}\n"
        "tt.stop();\n"
        "if (ret != brpc::ResponseMerger::FAIL) {\n"
        "   _stub_handler->update_latency(tt.u_elapsed(), \"pack_merge\");\n"
        "}\n"
        "return ret;\n");
    printer->Outdent();
    printer->Print("}\n");

    printer->Outdent();
    printer->Print("};\n");
    return true;
  }
  bool generate_paddle_serving_stub_default_map(
      google::protobuf::io::Printer* printer,
      const ServiceDescriptor* descriptor,
      string* error,
      const std::string& service_name,
      const std::string& class_name) const {
    printer->Print(
        "if (channel_index > 0) { \n"
        "   return brpc::SubCall::Skip();\n"
        "}\n");
    printer->Print(
        "google::protobuf::Message* cur_res = "
        "_stub_handler->fetch_response();\n"
        "if (cur_res == NULL) {\n"
        "   LOG(INFO) << \"Failed fetch response from stub handler, new it\";\n"
        "   cur_res = response->New();\n"
        "   if (cur_res == NULL) {\n"
        "       LOG(ERROR) << \"Failed new response item!\";\n"
        "       _stub_handler->update_average(1, \"pack_fail\");\n"
        "       return brpc::SubCall::Bad();\n"
        "   }\n"
        "   return brpc::SubCall(method, request, cur_res, "
        "brpc::DELETE_RESPONSE);\n"
        "}\n");
    "LOG(INFO) \n"
    "   << \"[default] Succ map, channel_index: \" << channel_index;\n";
    printer->Print("return brpc::SubCall(method, request, cur_res, 0);\n");
    return true;
  }
  bool generate_paddle_serving_stub_default_merger(
      google::protobuf::io::Printer* printer,
      const ServiceDescriptor* descriptor,
      string* error,
      const std::string& service_name,
      const std::string& class_name) const {
    printer->Print(
        "try {\n"
        "   response->MergeFrom(*sub_response);\n"
        "   return brpc::ResponseMerger::MERGED;\n"
        "} catch (const std::exception& e) {\n"
        "   LOG(ERROR) << \"Merge failed.\";\n"
        "   _stub_handler->update_average(1, \"pack_fail\");\n"
        "   return brpc::ResponseMerger::FAIL;\n"
        "}\n");
    return true;
  }
  bool generate_paddle_serving_stub_package_map(
      google::protobuf::io::Printer* printer,
      const ServiceDescriptor* descriptor,
      string* error,
      const std::string& service_name,
      const std::string& class_name,
      std::vector<const FieldDescriptor*>& in_shared_fields,        // NOLINT
      std::vector<const FieldDescriptor*>& in_item_fields) const {  // NOLINT
    const MethodDescriptor* md = descriptor->FindMethodByName("inference");
    if (!md) {
      *error = "not found inference method!";
      return false;
    }

    printer->Print(
        "const $req_type$* req \n"
        "       = dynamic_cast<const $req_type$*>(request);\n"
        "$req_type$* sub_req = NULL;",
        "req_type",
        google::protobuf::dots_to_colons(md->input_type()->full_name()));

    // 1. pack fields 逐字段计算index范围，并从req copy值sub_req
    printer->Print("\n// 1. 样本字段(必须为repeated类型)按指定下标复制\n");
    for (uint32_t ii = 0; ii < in_item_fields.size(); ii++) {
      const FieldDescriptor* fd = in_item_fields[ii];
      std::string field_name = fd->name();
      printer->Print("\n/////$field_name$\n", "field_name", field_name);
      if (ii == 0) {
        printer->Print(
            "uint32_t total_size = req->$field_name$_size();\n"
            "if (channel_index == 0) {\n"
            "   _stub_handler->update_average(total_size, \"item_size\");\n"
            "}\n",
            "field_name",
            field_name);

        printer->Print(
            "int start = _package_size * channel_index;\n"
            "if (start >= total_size) {\n"
            "   return brpc::SubCall::Skip();\n"
            "}\n"
            "int end = _package_size * (channel_index + 1);\n"
            "if (end > total_size) {\n"
            "   end = total_size;\n"
            "}\n");

        printer->Print(
            "sub_req = "
            "dynamic_cast<$req_type$*>(_stub_handler->fetch_request());\n"
            "if (sub_req == NULL) {\n"
            "    LOG(ERROR) << \"failed fetch sub_req from stub.\";\n"
            "    _stub_handler->update_average(1, \"pack_fail\");\n"
            "    return brpc::SubCall::Bad();\n"
            "}\n",
            "name",
            class_name,
            "req_type",
            google::protobuf::dots_to_colons(md->input_type()->full_name()));

      } else {
        printer->Print(
            "if (req->$field_name$_size() != total_size) {\n"
            "    LOG(ERROR) << \"pack field size not consistency: \"\n"
            "               << total_size << \"!=\" << "
            "req->$field_name$_size()\n"
            "               << \", field: $field_name$.\";\n"
            "    _stub_handler->update_average(1, \"pack_fail\");\n"
            "    return brpc::SubCall::Bad();\n"
            "}\n",
            "field_name",
            field_name);
      }

      printer->Print("for (uint32_t i = start; i < end; ++i) {\n");
      printer->Indent();
      if (fd->cpp_type() ==
          google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
        printer->Print(
            "sub_req->add_$field_name$()->CopyFrom(req->$field_name$(i));\n",
            "field_name",
            field_name);
      } else {
        printer->Print("sub_req->add_$field_name$(req->$field_name$(i));\n",
                       "field_name",
                       field_name);
      }
      printer->Outdent();
      printer->Print("}\n");
    }

    // 2. shared fields逐字段从req copy至sub_req
    printer->Print("\n// 2. 共享字段，从req逐个复制到sub_req\n");
    if (in_item_fields.size() == 0) {
      printer->Print(
          "if (sub_req == NULL) { // no packed items\n"
          "   sub_req = "
          "dynamic_cast<$req_type$*>(_stub_handler->fetch_request());\n"
          "   if (!sub_req) {\n"
          "       LOG(ERROR) << \"failed fetch sub_req from stub handler.\";\n"
          "       _stub_handler->update_average(1, \"pack_fail\");\n"
          "       return brpc::SubCall::Bad();\n"
          "   }\n"
          "}\n",
          "req_type",
          google::protobuf::dots_to_colons(md->input_type()->full_name()));
    }
    for (uint32_t si = 0; si < in_shared_fields.size(); si++) {
      const FieldDescriptor* fd = in_shared_fields[si];
      std::string field_name = fd->name();
      printer->Print("\n/////$field_name$\n", "field_name", field_name);
      if (fd->cpp_type() ==
              google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE ||
          fd->is_repeated()) {
        printer->Print(
            "sub_req->mutable_$field_name$()->CopyFrom(req->$field_name$());\n",
            "field_name",
            field_name);
      } else {
        printer->Print("sub_req->set_$field_name$(req->$field_name$());\n",
                       "field_name",
                       field_name);
      }
    }

    printer->Print(
        "LOG(INFO)\n"
        "   << \"[pack] Succ map req at: \"\n"
        "   << channel_index;\n");
    printer->Print(
        "google::protobuf::Message* sub_res = "
        "_stub_handler->fetch_response();\n"
        "if (sub_res == NULL) {\n"
        "    LOG(ERROR) << \"failed create sub_res from res.\";\n"
        "    _stub_handler->update_average(1, \"pack_fail\");\n"
        "    return brpc::SubCall::Bad();\n"
        "}\n"
        "return brpc::SubCall(method, sub_req, sub_res, 0);\n");
    return true;
  }
  bool generate_paddle_serving_stub_package_merger(
      google::protobuf::io::Printer* printer,
      const ServiceDescriptor* descriptor,
      string* error,
      const std::string& service_name,
      const std::string& class_name) const {
    return generate_paddle_serving_stub_default_merger(
        printer, descriptor, error, service_name, class_name);
  }
  bool generate_paddle_serving_stub_body(google::protobuf::io::Printer* printer,
                                         const ServiceDescriptor* descriptor,
                                         string* error,
                                         const std::string& service_name,
                                         const std::string& class_name) const {
    std::vector<const MethodDescriptor*> methods;
    for (int i = 0; i < descriptor->method_count(); ++i) {
      methods.push_back(descriptor->method(i));
    }
    if (!valid_service_method(methods)) {
      *error = "Service can only contains two methods: inferend, debug";
      return false;
    }

    const MethodDescriptor* md = methods[0];
    std::map<string, string> variables;
    variables["name"] = class_name;
    variables["req_type"] =
        google::protobuf::dots_to_colons(md->input_type()->full_name());
    variables["res_type"] =
        google::protobuf::dots_to_colons(md->output_type()->full_name());
    variables["fullname"] = descriptor->full_name();
    printer->Print(variables,
                   "REGIST_STUB_OBJECT_WITH_TAG(\n"
                   "       $name$_Stub,\n"
                   "       $name$_StubCallMapper,\n"
                   "       $name$_StubResponseMerger,\n"
                   "       $req_type$,\n"
                   "       $res_type$,\n"
                   "       \"$fullname$\");\n");
    variables.clear();
    return true;
  }
};
#endif  // #ifdef BCLOUD

int main(int argc, char** argv) {
  PdsCodeGenerator generator;
  return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
