set -x
set -v
function get_model(){
if [ ! -d "bert_cased_L-12_H-768_A-12_model" ]; then
    python -c "from paddle_serving_app.models import ServingModels; models = ServingModels();\
    models.download(\"$1\")"
tar -xzf $1.tar.gz
fi
}

function bert_demo(){
cd ../python/examples/bert
python prepare_model.py bert_chinese_L-12_H-768_A-12 20
sh benchmark.sh bert_chinese_L-12_H-768_A-12_seq20_model bert_chinese_L-12_H-768_A-12_seq20_client

python prepare_model.py ernie_tiny 20
sh benchmark.sh ernie_tiny_seq20_model ernie_tiny_seq20_client
cd -
}

function imagenet_demo(){
cd ../python/examples/imagenet
sh get_model.sh

}

bert_demo
