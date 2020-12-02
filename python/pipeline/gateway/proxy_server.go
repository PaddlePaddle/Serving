// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

package main

import (
  "C"
  "flag"
  "net/http"
  "log"
  "strconv"

  "golang.org/x/net/context"
  "github.com/grpc-ecosystem/grpc-gateway/runtime"
  "google.golang.org/grpc"

  gw "serving-gateway/proto"
)

//export run_proxy_server
func run_proxy_server(grpc_port int, http_port int) error {
  var (
    pipelineEndpoint = flag.String("pipeline_endpoint", "localhost:" + strconv.Itoa(grpc_port), "endpoint of PipelineService")
  )

  ctx := context.Background()
  ctx, cancel := context.WithCancel(ctx)
  defer cancel()

  //EmitDefaults=true, does not filter out the default inputs 
  mux := runtime.NewServeMux(runtime.WithMarshalerOption(runtime.MIMEWildcard, &runtime.JSONPb{OrigName: true, EmitDefaults: true}))
  opts := []grpc.DialOption{grpc.WithInsecure()}
  err := gw.RegisterPipelineServiceHandlerFromEndpoint(ctx, mux, *pipelineEndpoint, opts)
  if err != nil {
    return err
  }

  log.Println("start proxy service")
  return http.ListenAndServe(":" + strconv.Itoa(http_port), mux) // proxy port
}

func main() {}
