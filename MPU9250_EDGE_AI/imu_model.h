#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class DecisionTree {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        if (x[2] <= 0.024999999441206455) {
                            if (x[1] <= 0.08499999903142452) {
                                if (x[2] <= 0.004999999888241291) {
                                    if (x[0] <= -0.10500000230967999) {
                                        if (x[1] <= 0.03499999921768904) {
                                            if (x[2] <= -0.03499999921768904) {
                                                return 2;
                                            }

                                            else {
                                                if (x[1] <= -0.049999999813735485) {
                                                    return 1;
                                                }

                                                else {
                                                    return 2;
                                                }
                                            }
                                        }

                                        else {
                                            if (x[2] <= -0.06999999843537807) {
                                                return 1;
                                            }

                                            else {
                                                return 2;
                                            }
                                        }
                                    }

                                    else {
                                        return 3;
                                    }
                                }

                                else {
                                    if (x[1] <= -0.024999999441206455) {
                                        if (x[0] <= -0.2800000011920929) {
                                            return 1;
                                        }

                                        else {
                                            return 3;
                                        }
                                    }

                                    else {
                                        if (x[1] <= 0.03499999921768904) {
                                            if (x[0] <= -0.19500000029802322) {
                                                return 2;
                                            }

                                            else {
                                                return 1;
                                            }
                                        }

                                        else {
                                            if (x[0] <= -0.2150000035762787) {
                                                return 3;
                                            }

                                            else {
                                                return 2;
                                            }
                                        }
                                    }
                                }
                            }

                            else {
                                return 3;
                            }
                        }

                        else {
                            if (x[1] <= 0.03499999921768904) {
                                if (x[0] <= -0.1600000038743019) {
                                    if (x[2] <= 0.03499999921768904) {
                                        if (x[1] <= 0.019999999552965164) {
                                            return 2;
                                        }

                                        else {
                                            if (x[0] <= -0.1900000050663948) {
                                                return 0;
                                            }

                                            else {
                                                return 1;
                                            }
                                        }
                                    }

                                    else {
                                        if (x[1] <= 0.014999999664723873) {
                                            if (x[2] <= 0.07499999925494194) {
                                                return 2;
                                            }

                                            else {
                                                return 1;
                                            }
                                        }

                                        else {
                                            if (x[1] <= 0.024999999441206455) {
                                                if (x[2] <= 0.23500000312924385) {
                                                    return 2;
                                                }

                                                else {
                                                    return 1;
                                                }
                                            }

                                            else {
                                                return 1;
                                            }
                                        }
                                    }
                                }

                                else {
                                    if (x[1] <= 0.004999999888241291) {
                                        if (x[0] <= -0.13499999791383743) {
                                            if (x[1] <= -0.029999999329447746) {
                                                if (x[0] <= -0.14500000327825546) {
                                                    return 3;
                                                }

                                                else {
                                                    return 1;
                                                }
                                            }

                                            else {
                                                return 3;
                                            }
                                        }

                                        else {
                                            return 3;
                                        }
                                    }

                                    else {
                                        if (x[0] <= -0.14500000327825546) {
                                            return 1;
                                        }

                                        else {
                                            return 2;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[2] <= 0.07499999925494194) {
                                    if (x[2] <= 0.06499999947845936) {
                                        return 1;
                                    }

                                    else {
                                        if (x[0] <= -0.19500000029802322) {
                                            return 3;
                                        }

                                        else {
                                            if (x[0] <= -0.17500000447034836) {
                                                return 1;
                                            }

                                            else {
                                                return 3;
                                            }
                                        }
                                    }
                                }

                                else {
                                    return 1;
                                }
                            }
                        }
                    }

                protected:
                };
            }
        }
    }
