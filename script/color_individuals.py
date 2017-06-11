class Cost:
    def __init__(self, value=0, debug=None, color=None, previous_color=None):
        self.value = value
        self.color = color
        self.previous_color = previous_color
        if debug is None:
            self.debug = []
        elif isinstance(debug, list):
            self.debug = debug
        else:
            self.debug = [debug]

    def __add__(a, b):
        return Cost(
            value=a.value + b.value,
            debug=a.debug + b.debug,
            color=b.color if a.color is None else a.color,
            previous_color=b.previous_color if a.previous_color is None else a.previous_color,
        )

    def __lt__(a, b):
        if a.value != b.value:
            return a.value < b.value
        if a.color != b.color and a.color is not None and b.color is not None:
            return a.color < b.color
        if a.previous_color != b.previous_color and a.previous_color is not None and b.previous_color is not None:
            return a.previous_color < b.previous_color
        return False

    def __str__(self):
        return " ".join(filter(lambda x: x is not None, [
            "%d"%self.value,
            "c%s"%self.color if self.color is not None else None,
            "pc%s"%self.previous_color if self.previous_color is not None else None,
            " ".join(self.debug),
        ]))

def color_individuals(tgi, tg_color, sw=1, ab=1, vi=1, only_individual=None):
    # Index group by (individual, time)
    it_group = {}
    for t, g, i in tgi:
        it_group[i, t] = g

    if only_individual is None:
        individuals = sorted(set([i for t, g, i in tgi]))
    elif isinstance(only_individual, list):
        individuals = only_individual
    else:
        individuals = [only_individual]

    times = sorted(set([t for t, g, i in tgi]))
    t_group_colors = {}
    for t in times:
        t_group_colors[t] = set(tg_color[t].values())
    prev_t = {times[idx+1]: times[idx] for idx in range(len(times) - 1)}

    # Color the individuals.
    itc_min_cost = {}    # map: (i, t, c) -> min Cost
    it_color = {}
    total_min_cost = 0
    for i in individuals:
        # Find colors.
        colors = set([0])
        for t in times:
            if (i, t) not in it_group:
                continue
            g = it_group[i, t]
            colors.add(tg_color[t][g])
        # Compute the coloring cost.
        for t in times:
            if (i, t) in it_group:
                g = it_group[i, t]
                gc = tg_color[t][g]
            else:
                g = None
                gc = None
            for c in colors:
                cost = Cost(color=c)
                if t in prev_t:
                    previous_costs = []
                    for pc in colors:
                        pcost = itc_min_cost[i, prev_t[t], pc]
                        if pc != c:
                            pcost += Cost(sw, "sw")
                        previous_costs.append(pcost)
                    min_pcost = min(previous_costs)
                    cost += min_pcost
                    cost.previous_color = min_pcost.color
                if c != gc:
                    if gc != None:
                        cost += Cost(vi, "vi")
                    if c in t_group_colors[t]:
                        cost += Cost(ab, "ab")
                itc_min_cost[i, t, c] = cost
        # Trace back.
        min_color = None
        for t in reversed(times):
            if min_color is None:
                min_cost = min(itc_min_cost[i, times[-1], c] for c in colors)
                total_min_cost += min_cost.value
            else:
                min_cost = itc_min_cost[i, t, min_color]
            it_color[i, t] = min_cost.color
            min_color = min_cost.previous_color
    return total_min_cost, it_color

