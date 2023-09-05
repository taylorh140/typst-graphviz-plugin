#let wasm = plugin("./dot.wasm")


#let render(code) = {
  return str(wasm.render(bytes(code)))
}

#let render-svg(code, width: auto, height: auto, alt: none, fit: "cover") = {
  image.decode(render(code), format: "svg", width: width, height: height, alt: alt, fit: fit)
}

#show raw.where(lang: "dot-render"): it => render-svg(it.text)



```dot-render
digraph structs {
    node [shape=record];
    struct1 [label="<f0> left|<f1> mid&#92; dle|<f2> right"];
    struct2 [label=<{<B>bold text</B>| regular text | d }>];
    struct3 [label="hello&#92;nworld |{ b |{c|<here> b|e}| f}| g | h"];
    struct1:f1 -> struct2:f0;
    struct1:f2 -> struct3:here;
}
```