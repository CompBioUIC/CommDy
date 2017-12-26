
def expand(tgis):
    def expand_i(t, g, eyes):
        if isinstance(eyes, list):
            for i in eyes:
                yield t, g, i
        else:
            yield t, g, eyes
    def expand_gi(t, gis):
        if isinstance(gis, dict):
            raise ValueError('dict is not supported')
        elif isinstance(gis, list):
            for g, eyes in gis:
                yield from expand_i(t, g, eyes)
        else:
            g, eyes = gis
            yield from expand_i(t, g, eyes)
    def expand_tgi(tgis):
        if isinstance(tgis, dict):
            raise ValueError('dict is not supported')
        elif isinstance(tgis, list):
            for tgi in tgis:
                if len(tgi) == 3:
                    t, g, eyes = tgi
                    yield from expand_i(t, g, eyes)
                elif len(tgi) == 2:
                    t, gis = tgi
                    yield from expand_gi(t, gis)
    return list(expand_tgi(tgis))

