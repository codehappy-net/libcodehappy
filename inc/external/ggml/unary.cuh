#pragma once
#include "common.cuh"

#define CUDA_GELU_BLOCK_SIZE 256
#define CUDA_SILU_BLOCK_SIZE 256
#define CUDA_TANH_BLOCK_SIZE 256
#define CUDA_RELU_BLOCK_SIZE 256
#define CUDA_HARDSIGMOID_BLOCK_SIZE 256
#define CUDA_HARDSWISH_BLOCK_SIZE 256
#define CUDA_SQR_BLOCK_SIZE 256

void ggml_cuda_op_gelu(ggml_backend_cuda_context & ctx, ggml_tensor * dst);

void ggml_cuda_op_silu(ggml_backend_cuda_context & ctx, ggml_tensor * dst);

void ggml_cuda_op_gelu_quick(ggml_backend_cuda_context & ctx, ggml_tensor * dst);

void ggml_cuda_op_tanh(ggml_backend_cuda_context & ctx, ggml_tensor * dst);

void ggml_cuda_op_relu(ggml_backend_cuda_context & ctx, ggml_tensor * dst);

void ggml_cuda_op_hardsigmoid(ggml_backend_cuda_context & ctx, ggml_tensor * dst);

void ggml_cuda_op_hardswish(ggml_backend_cuda_context & ctx, ggml_tensor * dst);

void ggml_cuda_op_leaky_relu(ggml_backend_cuda_context & ctx, ggml_tensor * dst);

void ggml_cuda_op_sqr(ggml_backend_cuda_context & ctx, ggml_tensor * dst);
