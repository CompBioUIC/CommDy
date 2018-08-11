import pandas
import pathcover
import metrics
import sys

if len(sys.argv) != 3 and len(sys.argv) != 6:
    print("usage: %s <csv file> <output stats file> [<switching cost> <absence cost> <visiting cost>]"%sys.argv[0])
    exit(255)

if len(sys.argv) >= 3:
    input_file, output_file = sys.argv[1:3]
if len(sys.argv) >= 6:
    sw, ab, vi = map(int, sys.argv[3:6])
else:
    sw, ab, vi = 1, 1, 1

print(sw, ab, vi)
df = pandas.read_csv(input_file)
cdf = pathcover.color_dataframe(df, sw=sw, ab=ab, vi=vi)
m = metrics.compute_individual_metrics(cdf)
m.to_csv(output_file)

