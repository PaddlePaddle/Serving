package main

import (
  "flag"
  "net/http"
  "log"

  "github.com/golang/glog"
  "golang.org/x/net/context"
  "github.com/grpc-ecosystem/grpc-gateway/runtime"
  "google.golang.org/grpc"

  gw "./gateway"
)

var (
  pipelineEndpoint = flag.String("pipeline_endpoint", "localhost:18080", "endpoint of PipelineService")
)

func run() error {
  ctx := context.Background()
  ctx, cancel := context.WithCancel(ctx)
  defer cancel()

  mux := runtime.NewServeMux()
  opts := []grpc.DialOption{grpc.WithInsecure()}
  err := gw.RegisterPipelineServiceHandlerFromEndpoint(ctx, mux, *pipelineEndpoint, opts)
  if err != nil {
    return err
  }

  log.Println("start service")
  return http.ListenAndServe(":8080", mux) // proxy port
}

func main() {
  flag.Parse()
  defer glog.Flush()

  if err := run(); err != nil {
    glog.Fatal(err)
  }
}
