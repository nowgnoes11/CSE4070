#define P 17
#define Q 14
#define FRACTION (1 << Q)

#define ISF(i, f) ((i) * (FRACTION) - (f))
#define IMF(i, f) ((i) * (f))

#define FAI(f, i) ((f) + (i) * (FRACTION))
#define FDI(f, i) ((f) / (i))

#define FAF(f1, f2) ((f1) + (f2))
#define FSF(f1, f2) ((f1) - (f2))
#define FMF(f1, f2) (((int64_t)f1) * (f2) / (FRACTION))
#define FDF(f1, f2) (((int64_t)f1) * (FRACTION) / (f2))
