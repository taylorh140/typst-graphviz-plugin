#!/bin/python3


from pathlib import Path
import re

wat = Path('Pre-patched-dot.wat').read_text()

types={}
for _ in re.findall(r"\s*\(type \(;(\d+);\) \(func ?(\(param ([^\)]+)\))? ?(\(result ([^\)]+)\))?\)\)",wat):
    types[_[0]]=(_[1],_[3],_[4])

for _ in re.findall(r"(\(import [^\(]+\(func \(;(\d+);\) \(type (\d+)\)\)\))",wat):

    rep=f"(func (;{_[1]};) {types[_[2]][0]} {types[_[2]][1]} {'' if types[_[2]][2]=='' else types[_[2]][2]+'.const 0 return '} )"

    if "\"typst_env\"" in _[0]:
        continue

    if _[0] in wat:
        print("replacing:",_[0],"with",rep)
    wat=wat.replace(_[0],rep)


Path('dot.wat').write_text(wat)