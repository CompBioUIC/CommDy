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
            my_group_color = row['group_color']
            my_color = row['individual_color']
            group_colors = time_group_colors[row['time']]
            if my_group_color != my_color and my_color in group_colors:
                # If I am not in the group of my color, I am absent from the
                # group of my color.
                num_absent += 1
        values.append(float(num_absent) / len(group))
    return values

def inquisitiveness(df):
    values = []
    for individual, group in df.groupby(['individual']):
        num_visit = 0
        for index, row in group.iterrows():
            my_group_color = row['group_color']
            my_color = row['individual_color']
            if my_group_color != my_color and pd.notna(my_group_color):
                print("ind", individual, "visit")
                print(row)
                print()
                num_visit += 1
        values.append(float(num_visit) / len(group.dropna()))
    return values

def community_stay(df):
    #comm.stay = mean(compute.stay(na.omit(x)))
    pass

def peer_synchrony(df):
    #my.peers = merge(x, ig.color, by=c('time', 'i.color'), all.x=T, suffixes=c("", ".y"))
    #peer = mean(ddply(my.peers, .(time), function(y) sum(y$ind!=y$ind.y))[,2])
    #peer.synchrony = mean(ddply(my.peers, .(time), function(y) {
    #    z = with(y[y$ind!=y$ind.y,], next.i.color==next.i.color.y)
    #    ifelse (length(z)==0, 0, z)
    #})[,2])
    pass

def group_size(df):
    ##group.size = mean(merge(x, ddply(gtm, "group", nrow))$V1)
    pass

def individual_apparency(df):
    #ind.apparancy = nrow(na.omit(x))/nrow(x)
    pass

def cyclickty(df):
    #cyclicity = sum(table(x$i.color[which(c(T, diff(x$i.color)!=0))])-1)
    pass

def community_size(df):
    ##comm.size = mean(ddply(merge(x, ig.color, by=c("time", "i.color")), "time", nrow)$V1)
    pass

def individual_number_of_communities(df):
    #ind.num.comm = length(unique(x$i.color))
    pass

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

