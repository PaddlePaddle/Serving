import os
import sys
from paddle_serving_server import OpMaker
from paddle_serving_server import OpSeqMaker
from paddle_serving_server import Server



op_maker = OpMaker()

service_inputs = ["C_{}".format(i) for i in range(27)]
read_op = op_maker.create('general_reader', outputs=service_inputs)

kv_outputs = ["C_{}_embedding".format(i) for i in range(27)]
dist_kv_op = op_maker.create('general_dist_kv', inputs=service_inputs, outputs=[kv_outputs])

general_infer_op = op_maker.create('general_infer', inputs=[kv_outputs, "dense"], outputs=["prediction"])
response_op = op_maker.create('general_response', inputs=["prediction"])

op_seq_maker = OpDAGMaker()

op_seq_maker = OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(general_infer_op)
op_seq_maker.add_op(response_op)


server = RPCService()
server.set_op_sequence(op_seq_maker.get_op_sequence())
server.load_model_config(sys.argv[1])
server.prepare_server(workdir="work_dir1", port=9292, device="cpu")

imdb_service = WebService(name="imdb", model)
imdb_service.prepare_service({"dict_file_path":sys.argv[2]})
imdb_service.set_wrap_service(serving_service)
imdb_service.run_server()
