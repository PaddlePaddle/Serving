package io.paddle.serving.client;

import io.paddle.serving.grpc.*;

public class App 
{
    public static void main( String[] args )
    {
        System.out.println( "Hello World!" );
        SimpleResponse tmp = SimpleResponse.newBuilder()
            .setErrCode(1)
            .build();
        System.out.println( "Hello World!" );
    }
}
