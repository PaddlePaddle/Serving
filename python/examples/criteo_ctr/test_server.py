import os
import sys
from paddle_serving_server import OpMaker
from paddle_serving_server import OpSeqMaker
from paddle_serving_server import Server

op_maker = OpMaker()
read_op = op_maker.create('general_reader')
dist_op = op_maker.create('general_dist_kv')
general_infer_op = op_maker.create('general_infer')
response_op = op_maker.create('general_response')

op_seq_maker = OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(dist_op)
op_seq_maker.add_op(general_infer_op)
op_seq_maker.add_op(response_op)

server = Server()
server.set_op_sequence(op_seq_maker.get_op_sequence())
server.load_model_config(sys.argv[1])
server.prepare_server(workdir="work_dir1", port=9292, device="cpu")
server.run_server()
