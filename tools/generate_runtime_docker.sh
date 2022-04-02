#!/bin/sh

#abort on error
set -e

function usage
{
    echo "usage: sh tools/generate_runtime_docker.sh --SOME_ARG ARG_VALUE"
    echo "   ";
    echo "   --env                 : running env, cpu/cuda10.1/cuda10.2/cuda11.2";
    echo "   --python              : python version, 3.6/3.7/3.8/3.9 ";
    echo "   --serving             : serving version(v0.8.3/0.7.0)";
    echo "   --paddle              : paddle version(2.2.2/2.2.0)"
    echo "   --image_name          : image name(default serving_runtime:env-python)"
    echo "  -h | --help            : helper";
}

function parse_args
{
  # positional args
  args=()

  # named args
  while [ "$1" != "" ]; do
      case "$1" in
          --env )               env="$2";             shift;;
          --python )            python="$2";     shift;;
          --serving )           serving="$2";      shift;;
          --paddle )            paddle="$2";      shift;;
      --image_name )          image_name="$2";    shift;;
          -h | --help )         usage;            exit;; # quit and show usage
          * )                 args+=("$1")             # if no match, add it to the positional args
      esac
      shift # move to next kv pair
  done
  # restore positional args
  set -- "${args[@]}"

  # set positionals to vars
  positional_1="${args[0]}"
  positional_2="${args[1]}"

  # validate required args
  if [[ -z "${paddle}" || -z "${env}" || -z "${python}" || -z "${serving}" ]]; then
      echo "Invalid arguments. paddle or env or python or serving is missing."
      usage
      exit;
  fi

  if [[ -z "${image_name}" ]]; then
      image_name="serving_runtime:$env-$python"
      echo "image_name is not assigned, so it will be set ($image_name)."
  fi

}


function run
{
  parse_args "$@"

  echo "named arg: env: $env"
  if [ $env == "cpu" ]; then
      base_image="ubuntu:16.04"
  elif [ $env == "cuda10.1" ]; then
      base_image="nvidia\/cuda:10.1-cudnn7-runtime-ubuntu16.04"
  elif [ $env == "cuda10.2" ]; then
      base_image="nvidia\/cuda:10.2-cudnn8-runtime-ubuntu16.04"
  elif [ $env == "cuda11.2" ]; then
      base_image="nvidia\/cuda:11.2.0-cudnn8-runtime-ubuntu16.04"
  fi
  #python="2.2.0"
  #serving="0.7.0"
  echo "base image: $base_image"
  echo "named arg: python: $python"
  echo "named arg: serving: $serving"
  echo "named arg: paddle: $paddle"
  echo "named arg: image_name: $image_name"
  
  sed -e "s/<<base_image>>/$base_image/g" -e "s/<<python_version>>/$python/g" -e "s/<<run_env>>/$env/g" -e "s/<<serving_version>>/$serving/g" -e "s/<<paddle_version>>/$paddle/g" tools/Dockerfile.runtime_template > Dockerfile.tmp
  #docker build --network=host --build-arg ftp_proxy=http://172.19.57.45:3128 --build-arg https_proxy=http://172.19.57.45:3128 --build-arg http_proxy=http://172.19.57.45:3128 --build-arg HTTP_PROXY=http://172.19.57.45:3128 --build-arg HTTPS_PROXY=http://172.19.57.45:3128 -t $image_name -f Dockerfile.tmp .
  docker build --network=host -t $image_name -f Dockerfile.tmp .
}

run "$@";
