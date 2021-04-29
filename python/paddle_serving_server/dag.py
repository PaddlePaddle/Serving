
from .proto import server_configure_pb2 as server_sdk
import google.protobuf.text_format
import collections

class OpMaker(object):
    def __init__(self):
        self.op_dict = {
            "general_infer": "GeneralInferOp",
            "general_reader": "GeneralReaderOp",
            "general_response": "GeneralResponseOp",
            "general_text_reader": "GeneralTextReaderOp",
            "general_text_response": "GeneralTextResponseOp",
            "general_single_kv": "GeneralSingleKVOp",
            "general_dist_kv_infer": "GeneralDistKVInferOp",
            "general_dist_kv": "GeneralDistKVOp",
            "general_copy": "GeneralCopyOp",
            "general_detection":"GeneralDetectionOp",
        }
        self.node_name_suffix_ = collections.defaultdict(int)

    def create(self, node_type, engine_name=None, inputs=[], outputs=[]):
        if node_type not in self.op_dict:
            raise Exception("Op type {} is not supported right now".format(
                node_type))
        node = server_sdk.DAGNode()
        # node.name will be used as the infer engine name
        if engine_name:
            node.name = engine_name
        else:
            node.name = '{}_{}'.format(node_type,
                                       self.node_name_suffix_[node_type])
            self.node_name_suffix_[node_type] += 1

        node.type = self.op_dict[node_type]
        if inputs:
            for dep_node_str in inputs:
                dep_node = server_sdk.DAGNode()
                google.protobuf.text_format.Parse(dep_node_str, dep_node)
                dep = server_sdk.DAGNodeDependency()
                dep.name = dep_node.name
                dep.mode = "RO"
                node.dependencies.extend([dep])
        # Because the return value will be used as the key value of the
        # dict, and the proto object is variable which cannot be hashed,
        # so it is processed into a string. This has little effect on
        # overall efficiency.
        return google.protobuf.text_format.MessageToString(node)

class OpSeqMaker(object):
    def __init__(self):
        self.workflow = server_sdk.Workflow()
        self.workflow.name = "workflow1"
        self.workflow.workflow_type = "Sequence"

    def add_op(self, node_str):
        node = server_sdk.DAGNode()
        google.protobuf.text_format.Parse(node_str, node)
        if len(node.dependencies) > 1:
            raise Exception(
                'Set more than one predecessor for op in OpSeqMaker is not allowed.'
            )
        if len(self.workflow.nodes) >= 1:
            if len(node.dependencies) == 0:
                dep = server_sdk.DAGNodeDependency()
                dep.name = self.workflow.nodes[-1].name
                dep.mode = "RO"
                node.dependencies.extend([dep])
            elif len(node.dependencies) == 1:
                if node.dependencies[0].name != self.workflow.nodes[-1].name:
                    raise Exception(
                        'You must add op in order in OpSeqMaker. The previous op is {}, but the current op is followed by {}.'
                        .format(node.dependencies[0].name, self.workflow.nodes[
                            -1].name))
        self.workflow.nodes.extend([node])

    def get_op_sequence(self):
        workflow_conf = server_sdk.WorkflowConf()
        workflow_conf.workflows.extend([self.workflow])
        return workflow_conf

# TODO:Currently, SDK only supports "Sequence".OpGraphMaker is not useful.
# Config should be changed to adapt command-line for list[dict] or list[list[] ]
class OpGraphMaker(object):
    def __init__(self):
        self.workflow = server_sdk.Workflow()
        self.workflow.name = "workflow1"
        # Currently, SDK only supports "Sequence"
        self.workflow.workflow_type = "Sequence"

    def add_op(self, node_str):
        node = server_sdk.DAGNode()
        google.protobuf.text_format.Parse(node_str, node)
        self.workflow.nodes.extend([node])

    def get_op_graph(self):
        workflow_conf = server_sdk.WorkflowConf()
        workflow_conf.workflows.extend([self.workflow])
        return workflow_conf
