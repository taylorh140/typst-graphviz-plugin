#let wasm = plugin("./dot.wasm")


#let render(code) = {
  return str(wasm.render(bytes(code)))
}

#let render-svg(code, width: auto, height: auto, alt: none, fit: "cover") = {
  set text(font:"linux libertine",fill:black)
  image.decode(render(code), format: "svg", alt: alt, fit: fit)
  //render(code)
}

#show raw.where(lang: "dot-render"): it => render-svg(it.text)


```dot-render
digraph finite_state_machine {
  rankdir=LR;
  node [shape = doublecircle]; 0 3 4 8;
  node [shape = circle];
  0 -> 2 [label = "SS(B)"];
  0 -> 1 [label = "SS(S)"];
  1 -> 3 [label = "S($end)"];
  2 -> 6 [label = "SS(b)"];
  2 -> 5 [label = "SS(a)"];
  2 -> 4 [label = "S(A)"];
  5 -> 7 [label = "S(b)"];
  5 -> 5 [label = "S(a)"];
  6 -> 6 [label = "S(b)"];
  6 -> 5 [label = "S(a)"];
  7 -> 8 [label = "S(b)"];
  7 -> 5 [label = "S(a)"];
  8 -> 6 [label = "S(b)"];
  8 -> 5 [label = "S(a)"];
}
```

