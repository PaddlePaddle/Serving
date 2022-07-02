#!/bin/sh

#abort on error
set -e

function usage
{
    echo "usage: sh tools/generate_k8s_yaml.sh --SOME_ARG ARG_VALUE"
    echo "   ";
    echo "   --app_name         : app name";
    echo "   --image_name       : image name";
    echo "   --workdir          : workdir in image";
    echo "   --command          : command to launch serving"
    echo "   --port             : serving port"
    echo "   --pod_num          : number of pod replicas"
    echo "  -h | --help         : helper";
}

function parse_args
{
  # positional args
  args=()

  # default
  pod_num=1

  # named args
  while [ "$1" != "" ]; do
      case "$1" in
          --app_name )               app_name="$2";             shift;;
          --image_name )            image_name="$2";     shift;;
          --workdir )           workdir="$2";      shift;;
          --command )            start_command="$2";      shift;;
          --port )          port="$2";    shift;;
          --pod_num ) 	    pod_num="$2"; 	shift;;
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
  if [[ -z "${app_name}" || -z "${image_name}" || -z "${workdir}" || -z "${start_command}" || -z "${port}" || -z "${pod_num}"]]; then
      echo "Invalid arguments. check your params again."
      usage
      exit;
  fi

}


function run
{
  parse_args "$@"

  echo "named arg: app_name: $app_name"
  echo "named arg: image_name: $image_name"
  echo "named arg: workdir: $workdir"
  echo "named arg: command: $start_command"
  echo "named arg: port: $port"
  echo "named arg: pod_num: $pod_num"
  
  sed -e "s/<< APP_NAME >>/$app_name/g" -e "s/<< IMAGE_NAME >>/$(echo $image_name | sed -e 's/\\/\\\\/g; s/\//\\\//g; s/&/\\\&/g')/g" -e "s/<< WORKDIR >>/$(echo $workdir | sed -e 's/\\/\\\\/g; s/\//\\\//g; s/&/\\\&/g')/g" -e "s/<< COMMAND >>/\"$(echo $start_command | sed -e 's/\\/\\\\/g; s/\//\\\//g; s/&/\\\&/g')\"/g" -e "s/<< PORT >>/$port/g"  tools/k8s_serving.yaml_template > k8s_serving.yaml
  sed -e "s/<< APP_NAME >>/$app_name/g" -e "s/<< IMAGE_NAME >>/$(echo $image_name | sed -e 's/\\/\\\\/g; s/\//\\\//g; s/&/\\\&/g')/g" -e "s/<< WORKDIR >>/$(echo $workdir | sed -e 's/\\/\\\\/g; s/\//\\\//g; s/&/\\\&/g')/g" -e "s/<< COMMAND >>/\"$(echo $start_command | sed -e 's/\\/\\\\/g; s/\//\\\//g; s/&/\\\&/g')\"/g" -e "s/<< PORT >>/$port/g"  tools/k8s_ingress.yaml_template > k8s_ingress.yaml
  echo "check k8s_serving.yaml and k8s_ingress.yaml please." 
}

run "$@";
