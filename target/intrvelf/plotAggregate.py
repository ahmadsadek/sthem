#!/usr/bin/python3

import sys
import os
import argparse
import struct
import bz2
import pickle
import plotly
import plotly.graph_objs as go
import numpy

parser = argparse.ArgumentParser(description="Visualize profiles from intrvelf sampler.")
parser.add_argument("profile", help="postprocessed profile from intrvelf")
parser.add_argument("-o", "--output", help="html output file")
parser.add_argument("-q", "--quiet", action="store_true", help="do not automatically open output file")


args = parser.parse_args()

if (not args.profile) or (not os.path.isfile(args.profile)):
    print("ERROR: profile not found")
    parser.print_help()
    sys.exit(1)



if args.profile.endswith(".bz2"):
    profile = pickle.load(bz2.BZ2File(args.profile, mode="rb"))
else:
    profile = pickle.load(open(args.profile, mode="rb"))

latencyUs = int(profile['latencyTimeUs'] / profile['samples'])
sampleTime = profile['samplingTimeUs'] / (profile['samples'] * 1000000)
freq = 1/sampleTime

volts = 1 if (profile['volts'] == 0) else profile['volts']

#profile['aggregatedProfile'][addr] = [samples, current, function, file, line]

functions = [] 
currents = [] 
samples = []

for sample in profile['aggregatedProfile']:
    sample = profile['aggregatedProfile'][sample];
    if (sample[0] > 0):
        function = profile['functions'][sample[2]]
        if function in functions:
            currents[functions.index(function)] += sample[1]
            samples[functions.index(function)] += sample[0]
        else:
            functions.append(profile['functions'][sample[2]])
            currents.append(sample[1])
            samples.append(sample[0])

currents = [ ((x/y) * volts * (y * sampleTime)) for x,y in zip(currents,samples) ]
sortedZipped = numpy.array(sorted(zip(currents, functions, samples)))
currents = numpy.array(sortedZipped[:,0:1], dtype=float).flatten()
functions = sortedZipped[:,1:2].flatten()
labelUnit = "C" if profile['volts'] == 0 else "J"
labels = [ f"{x:.2f} us, {y:.2f} {labelUnit}" for x,y in zip((numpy.array(sortedZipped[:,2:3], dtype=int) * sampleTime).flatten(), currents) ]

functionLength = numpy.max([ len(x) for x in functions ])

fig = {
    "data" : [go.Bar(
        x=currents,
        y=functions,
        text=labels,
        textposition = 'auto',
        orientation='h',
        hoverinfo="x"
    )],
    "layout" : go.Layout(
        title=go.layout.Title(
            text = f"{profile['elf']}, {freq:.2f} Hz, {profile['samples']} samples, {latencyUs} us latency",
            xref='paper',
            x=0
        ),
        xaxis=go.layout.XAxis(
            title=go.layout.xaxis.Title(
                text = "Charge in C" if profile['volts'] == 0 else "Energy in J",
                font=dict(
                    family='Courier New, monospace',
                    size=18,
                    color='#7f7f7f'
                )
            )
        ),
        yaxis=go.layout.YAxis(
           tickfont=dict(
               family='monospace',
               size=12,
               color='black'
           ) 
        ),
        margin=go.layout.Margin(
            l = 7.5 * numpy.max([ len(x) for x in functions ])
        )
    )
}

file = "temp-plot.html" if not args.output else args.output

plotly.offline.plot(fig, filename=file, auto_open=not args.quiet)
print(f"Plot saved to {file}")
