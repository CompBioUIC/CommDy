import numpy as np
import pandas as pd

def column_map():
    return {
        'time': 'time',
        'group': 'group',
        'individual': 'individual',
        'group_color': 'group_color',
        'individual_color': 'individual_color',
    }

def absenteeism(df, colmap=column_map()):
    time_group_colors = {
            t: set(group['group_color'].values)
            for t, group in df[['time', 'group_color']].drop_duplicates().dropna().groupby('time')}
    values = []
    for individual, group in df.groupby(['individual']):
        num_absent = 0
        for index, row in group.iterrows():
            if row['group_color'] != row['individual_color'] and row['individual_color'] in time_group_colors[row['time']]:
                num_absent += 1
        values.append(float(num_absent) / len(group))
    return values

def compute_stats(df,
        t_col='time', g_col='group', i_col='individual',
        gcolor_col='gcolor', icolor_col='icolor'):
    times = np.unique(df[[t_col]].values.flat).tolist()
    groups = np.unique(df[[g_col]].values.flat).tolist()
    individuals = np.unique(df[[i_col]].values.flat).tolist()

    #times['key'] = 0
    #groups['key'] = 0
    #individuals['key'] = 0

    #pandas.merge(pandas.merge(times, groups, on='key'), individuals, on='key')

    #ig_color = relabel.ind.with.most.frequent.label(ig.color)

    #absenteeism = sum(x$absent)/nrow(na.omit(x))
    #inquisitiveness = with(na.omit(x), sum(i.color!=g.color))/nrow(na.omit(x))
    #comm.stay = mean(compute.stay(na.omit(x)))
    #my.peers = merge(x, ig.color, by=c('time', 'i.color'), all.x=T, suffixes=c("", ".y"))
    #peer = mean(ddply(my.peers, .(time), function(y) sum(y$ind!=y$ind.y))[,2])
    #peer.synchrony = mean(ddply(my.peers, .(time), function(y) {
    #    z = with(y[y$ind!=y$ind.y,], next.i.color==next.i.color.y)
    #    ifelse (length(z)==0, 0, z)
    #})[,2])
    ##group.size = mean(merge(x, ddply(gtm, "group", nrow))$V1)
    #ind.apparancy = nrow(na.omit(x))/nrow(x)
    #cyclicity = sum(table(x$i.color[which(c(T, diff(x$i.color)!=0))])-1)
    ##comm.size = mean(ddply(merge(x, ig.color, by=c("time", "i.color")), "time", nrow)$V1)
    #ind.num.comm = length(unique(x$i.color))

    ## old stuff
    #ind.time.span   = diff(range(na.omit(x)$time))/nrow(x)

