// A Simple g Program
.module "test"         ;
.prefix "test"         ;
.optimizer sgd 0.1     ;
.precision float       ;
.costfnc cross_entropy ;
.batch 8               ;
.input 28 * 28         ;
.output 10 softmax     ;
.hidden 100 relu       ;
.hidden 100 relu       ;
