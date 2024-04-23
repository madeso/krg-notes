
## Fake GI

Bounced light
Local GI fakery

```glsl
float l_dot_n = dot(light_dir, normal);
l_dot_n = l_dot_n * hardness  + 1 - hardness
```
We can fade of the normal completly if we'd like, giving us more ambient light.


