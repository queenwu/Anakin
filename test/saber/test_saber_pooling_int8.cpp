#include <vector>
#include <limits>

#include "saber/core/context.h"
#include "test/saber/test_saber_base.h"
#include "test_saber_func.h"
#include "saber/core/tensor_op.h"
#include "saber/saber_types.h"
#include "saber/funcs/pooling.h"
#include "saber/core/data_traits.h"
#if defined(USE_X86_PLACE)
#include "jit_generator.h"
#endif
using namespace anakin::saber;

template<DataType Dtype_IN, DataType Dtype_OUT, typename TargetType_D, typename TargetType_H>
void pooling_cpu_func(const std::vector<Tensor<TargetType_H>*>& input,
                      std::vector<Tensor<TargetType_H>*>& output,
                      PoolingParam<TargetType_D>& param) {
    typedef typename DataTrait<TargetType_D, Dtype_IN> :: Dtype dtype_in;
    typedef typename DataTrait<TargetType_D, Dtype_OUT> :: Dtype dtype_out;

    const dtype_in* src_ptr = static_cast<dtype_in*>(input[0]->data());
    dtype_out* dst_ptr = static_cast<dtype_out*>(output[0]->mutable_data());

    int in_n = input[0]->num();
    int in_c = input[0]->channel();
    int in_h = input[0]->height();
    int in_w = input[0]->width();
    int size_in_n = in_c * in_h * in_w;
    int size_in_c = 1;

    int out_h = output[0]->height();
    int out_w = output[0]->width();
    int size_out_n = in_c * out_h * out_w;
    int size_out_c = 1;

    for (int ind_n = 0; ind_n < in_n; ++ind_n) {
        for (int ind_h = 0; ind_h < out_h; ++ind_h) {
            int sh = ind_h * param.stride_h;
            int eh = sh + param.window_h;

            if (param.pad_h > 0) {
                sh = (sh - param.pad_h) < 0 ? 0 : sh - param.pad_h;
                eh = (eh - param.pad_h) > in_h ? in_h : eh - param.pad_h;
            }

            for (int ind_w = 0; ind_w < out_w; ++ind_w) {
                int sw = ind_w * param.stride_w;
                int ew = sw + param.window_w;

                if (param.pad_w > 0) {
                    sw = (sw - param.pad_w) < 0 ? 0 : sw - param.pad_w;
                    ew = (ew - param.pad_w) > in_w ? in_w : ew - param.pad_w;
                }

                float result = 0;

                for (int ind_c = 0; ind_c < in_c; ++ind_c) {
                    int dst_ind = ind_n * size_out_n + ind_h * out_w * in_c + ind_w * in_c + ind_c;

                    for (int kh = sh; kh < eh; ++kh) {
                        for (int kw = sw; kw < ew; ++kw) {
                            int src_ind = ind_n * size_in_n + kh * in_w * in_c + kw * in_c + ind_c;

                            if (kh == sh && kw == sw) {
                                result = src_ptr[src_ind];
                            } else {
                                if (param.pooling_type == Pooling_max) {
                                    result = result >= src_ptr[src_ind] ? result : src_ptr[src_ind];
                                }

                                if (param.pooling_type == Pooling_average_include_padding) {
                                    result += src_ptr[src_ind];
                                }

                                if (param.pooling_type == Pooling_average_exclude_padding) {
                                    result += src_ptr[src_ind];
                                }
                            }
                        }
                    }

                    if (param.pooling_type == Pooling_average_include_padding) {
                        result /= param.window_h * param.window_w;
                    }

                    if (param.pooling_type == Pooling_average_exclude_padding) {
                        result /= (ew - sw) * (eh - sh);
                    }

                    if (Dtype_OUT != AK_FLOAT) {
                        dst_ptr[dst_ind] = static_cast<dtype_out>(nearbyintf(result));
                    } else {
                        dst_ptr[dst_ind] = result;
                    }
                }
            }
        }
    }
}

//test template for different device and dtype
template <typename TargetType_D, typename TargetType_H, DataType Dtype_IN, DataType Dtype_OUT>
void test_pooling() {
    typedef typename DataTrait<TargetType_D, Dtype_IN> :: Dtype dtype_in;
    typedef typename DataTrait<TargetType_D, Dtype_OUT> :: Dtype dtype_out;
    TestSaberBase<TargetType_D, TargetType_H, AK_INT8, Pooling, PoolingParam> testbase;

    for (int window_h : {
                2, 4
            }) {
        for (int window_w : {
                    2, 4
                }) {
            for (int pad_h : {
                        0, 1
                    }) {
                for (int pad_w : {
                            0, 1
                        }) {
                    for (PoolingType pooling_type : {
                                Pooling_max, Pooling_average_include_padding, Pooling_average_exclude_padding
                            }) {
                        for (int stride_h : {
                                    1, 2
                                }) {
                            for (int stride_w : {
                                        1, 2
                                    }) {
                                PoolingParam<TargetType_D> param(window_h, window_w, pad_h, pad_w, stride_h, stride_w,
                                                                 pooling_type);
                                LOG(INFO) << "win_h:" << window_h << "win_w:" << window_w \
                                          << "pad_h:" << pad_h << "pad_w:" << pad_w \
                                          << "stride_h:" << stride_h << "stride_w:" << stride_w \
                                          << "pooling_type:" << pooling_type;

                                for (int in_n : {
                                            1, 2
                                        }) {
                                    for (int in_c : {
                                                1, 3, 8
                                            }) {
                                        for (int in_h : {
                                                    32, 64
                                                }) {
                                            for (int in_w : {
                                                        32, 64
                                                    }) {
                                                LOG(INFO) << "n:" << in_n << ",in_h:" << in_h << ",in_w:" << in_w << ",in_c:" << in_c;
                                                testbase.set_param(param);//set param
                                                testbase.set_input_datatype(Dtype_IN);
                                                testbase.set_input_shape(Shape({in_n, in_h, in_w, in_c}, Layout_NHWC),{1.f},{1.f});//add some input shape
                                                testbase.set_ouput_datatype(Dtype_OUT);
                                                testbase.run_test(pooling_cpu_func<Dtype_IN, Dtype_OUT, TargetType_D, TargetType_H>);//run test

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

TEST(TestSaberFunc, test_func_pool) {
#ifdef USE_X86_PLACE

    //    test_pooling<X86, X86, AK_UINT8, AK_UINT8>();
    //    test_pooling<X86, X86, AK_UINT8, AK_FLOAT>();
    if (jit::mayiuse(jit::avx512_core)) {
        test_pooling<X86, X86, AK_UINT8, AK_UINT8>();
//        test_pooling<X86, X86, AK_INT8, AK_INT8>();
    }

#endif
}

int main(int argc, const char** argv) {
    // initial logger
    logger::init(argv[0]);
    InitTest();
    RUN_ALL_TESTS(argv[0]);
    return 0;
}
