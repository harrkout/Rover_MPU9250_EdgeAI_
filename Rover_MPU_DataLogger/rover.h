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
                        if (x[2] <= 0.9799999743700027) {
                            if (x[2] <= -1.1449999809265137) {
                                return 4;
                            }

                            else {
                                if (x[0] <= -0.19500000029802322) {
                                    if (x[0] <= -0.20499999821186066) {
                                        if (x[2] <= 0.19500000029802322) {
                                            if (x[1] <= 0.024999999441206455) {
                                                return 0;
                                            }

                                            else {
                                                if (x[0] <= -0.24000000208616257) {
                                                    return 1;
                                                }

                                                else {
                                                    if (x[0] <= -0.22500000149011612) {
                                                        return 0;
                                                    }

                                                    else {
                                                        return 1;
                                                    }
                                                }
                                            }
                                        }

                                        else {
                                            return 1;
                                        }
                                    }

                                    else {
                                        if (x[1] <= 0.004999999888241291) {
                                            if (x[1] <= -0.014999999664723873) {
                                                return 0;
                                            }

                                            else {
                                                if (x[2] <= 0.03499999921768904) {
                                                    return 2;
                                                }

                                                else {
                                                    return 1;
                                                }
                                            }
                                        }

                                        else {
                                            if (x[1] <= 0.044999999925494194) {
                                                return 0;
                                            }

                                            else {
                                                return 1;
                                            }
                                        }
                                    }
                                }

                                else {
                                    if (x[1] <= -0.004999999888241291) {
                                        if (x[1] <= -0.024999999441206455) {
                                            return 0;
                                        }

                                        else {
                                            return 5;
                                        }
                                    }

                                    else {
                                        if (x[1] <= 0.019999999552965164) {
                                            if (x[1] <= 0.004999999888241291) {
                                                return 1;
                                            }

                                            else {
                                                return 5;
                                            }
                                        }

                                        else {
                                            return 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            return 3;
                        }
                    }

                protected:
                };
            }
        }
    }
