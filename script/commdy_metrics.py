import pandas
import pathcover
import metrics
import sys

if len(sys.argv) <= 2:
    print("usage: %s <csv file> <output stats file>"%sys.argv[0])
    exit(255)

df = pandas.read_csv(sys.argv[1])
cdf = pathcover.color_dataframe(df)
m = metrics.compute_individual_metrics(cdf)
m.to_csv(sys.argv[2])

