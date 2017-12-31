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

def adjacent_pairs(items):
    it = iter(items)
    prev = next(it)
    for curr in it:
        yield prev, curr
        prev = curr

def avg(items):
    return 0.0 if len(items) == 0 else sum(items) / float(len(items))

# absenteeism = % times an individual is absent from his/her community's groups.
def absenteeism(df, colmap=column_map()):
    time_group_colors = {
            t: set(group['group_color'].values)
            for t, group in df[['time', 'group_color']].drop_duplicates().dropna().groupby('time')}
    values = []
    for _, group in df.groupby(['individual']):
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

# inquisitiveness = % times an individual is present in groups other than those
# of his/her community.
def inquisitiveness(df):
    values = []
    for _, group in df.groupby(['individual']):
        num_visit = 0
        for index, row in group.iterrows():
            my_group_color = row['group_color']
            my_color = row['individual_color']
            if my_group_color != my_color and pd.notna(my_group_color):
                num_visit += 1
        values.append(float(num_visit) / len(group.dropna()))
    return values

# community_stay = average number of time steps an individual consecutively
#                  stays with the same community.
def community_stay(df):
    values = []
    for _, group in df.groupby(['individual']):
        xs = []
        my_colors = group['individual_color'].tolist()
        if len(my_colors) == 0:
            values.append(0.0)
            continue
        offset = 0
        for i in range(1, len(my_colors)):
            if my_colors[i] == my_colors[i-1]:
                continue
            xs.append(i - offset)
            offset = i
        xs.append(len(my_colors) - offset)
        values.append(avg(xs))
    return values

# A subgroup of a group contains the members of the same community.
def compute_subgroups(df):
    community_subgroups = {}
    for _, row in df.dropna().iterrows():
        t, g, i, c = [row[x] for x in ['time', 'group', 'individual', 'individual_color']]
        if (t, g, c) in community_subgroups:
            community_subgroups[t, g, c].add(i)
        else:
            community_subgroups[t, g, c] = set([i])
    return community_subgroups

# average number of group memebers other than itself that share the same
# community identity. The averaging is done over observed time steps.
def avg_num_peers(df, community_subgroups):
    values = []
    for _, group in df.groupby(['individual']):
        xs = []
        for _, row in group.dropna().iterrows():
            g = row['group']
            t = row['time']
            ic = row['individual_color']
            xs.append(len(community_subgroups[t, g, ic]) - 1)
        values.append(avg(xs))
    return values

# The number of peers (other members of the group with the same community
# identity) who were peers in the previous observed time step.
def peer_synchrony(df, community_subgroups):
    values = []
    for _, group in df.groupby(['individual']):
        xs = []
        for (_, prev_row), (_, curr_row) in adjacent_pairs(group.dropna().iterrows()):
            t1, g1, c1 = prev_row['time'], prev_row['group'], prev_row['individual_color']
            t2, g2, c2 = curr_row['time'], curr_row['group'], curr_row['individual_color']
            members1 = community_subgroups[t1, g1, c1]
            members2 = community_subgroups[t2, g2, c2]
            if len(members2) == 1:
                xs.append(0.0)
            else:
                xs.append((len(members1.intersection(members2)) - 1.0) / (len(members2) - 1.0))
        values.append(avg(xs))
    return values

def group_size(df):
    group_size = df.dropna().groupby(['time', 'group'])['individual'].count().to_dict()
    d = {
        'individual': df['individual'],
        'group_size': df.dropna().apply(lambda x: group_size[x['time'], x['group']], 1),
    }
    return pd.DataFrame(d).groupby('individual')['group_size'].mean().values.tolist()

def group_homogeneity(df):
    d = {
        'time' : df['time'],
        'group' : df['group'],
        'homogeneity': (df['group_color'] == df['individual_color']).astype(float),
    }
    h = pd.DataFrame(d).dropna().groupby(['time', 'group'])['homogeneity'].mean().to_dict()
    values = []
    for _, group in df.groupby('individual'):
        values.append(avg([h[row['time'], row['group']] for _, row in group.dropna().iterrows()]))
    return values

def individual_apparency(df):
    time_index = {t: i for i, t in enumerate(df['time'].unique())}
    def diff(t2, t1):
        return time_index[t2] - time_index[t1] + 1
    community_apparency = {ic: diff(max(group['time']), min(group['time']))
        for ic, group in df.groupby('individual_color')}
    values = []
    for _, group in df.groupby('individual'):
        values.append(avg([community_apparency[ic] for ic in set(group['individual_color'].values)]))
    return values

def cyclicity(df):
    # TODO: implemen this.
    pass

def community_size(df):
    # TODO: implemen this.
    pass

def individual_number_of_communities(df):
    # TODO: implemen this.
    pass

def compute_stats(df,
        t_col='time', g_col='group', i_col='individual',
        gcolor_col='gcolor', icolor_col='icolor'):
    # TODO: implement this.
