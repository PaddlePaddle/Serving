from paddle_serving_client.io import inference_model_to_serving


def serving_encryption():
    inference_model_to_serving(
        dirname="./DarkNet53/ppcls_model/",
        model_filename="__model__",
        params_filename="./__params__",
        serving_server="encrypt_server",
        serving_client="encrypt_client",
        encryption=True)


if __name__ == "__main__":
    serving_encryption()
