import os
import shutil
import argparse

def parse_args():
    parser = argparse.ArgumentParser(description="PaddlePaddle CTR example")
    parser.add_argument(
        '--model_dir',
        type=str,
        required=True,
        help='The trained model path (eg. models/pass-0)')
    parser.add_argument(
        '--inference_only_model_dir',
        type=str,
        required=True,
        help='The inference only model (eg. models/inference_only)')
    return parser.parse_args()

def replace_params():
    args = parse_args()
    inference_only_model_dir = args.inference_only_model_dir
    model_dir = args.model_dir

    files = [f for f in os.listdir(inference_only_model_dir)]

    for f in files:
        if (f.find("__model__") != -1):
            continue
        src_file = inference_only_model_dir + "/" + f
        dst_file = args.model_dir + "/" + f
        print("copying %s to %s" % (src_file, dst_file))
        shutil.copyfile(src_file, dst_file)

if __name__ == '__main__':
    replace_params()
    print("Done")
