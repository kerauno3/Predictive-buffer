# Predictive Buffer (Geode mod)

An input-buffer assist for Geometry Dash. If you click to jump a few physics
steps before you land, the click is remembered and fired on the first grounded
step, so slightly early inputs still make the jump.

This is an original implementation, not a copy of any other mod's code.

## Controls

Press **F6** to cycle the buffer window:

| Mode | Window (physics steps) | Approx. time at 240 steps/s |
|------|------------------------|-----------------------------|
| 1    | 10                     | ~41 ms                      |
| 2    | 5                      | ~21 ms                      |
| 3    | 2                      | ~8 ms                       |
| 4    | Off (default)          | -                           |

The mode is saved between sessions.

## Behavior

- Only applies to cube-style one-shot jumps (not ship/UFO/wave/swing hold-style modes).
- A click made while touching an orb is left alone, and a click consumed by an orb cancels the buffer.
- Setting **"Block leaderboard/stat submission"** (on by default): if the buffer
  fired during a run, completing the level is treated like a test-mode run, so
  an assisted run can't be submitted as a legit record. Use it for practice,
  casual play, and your own fun, not for passing off as unassisted.

## Building

You need the [Geode SDK](https://docs.geode-sdk.org/getting-started/) and the
Geode CLI.

```
geode build
```

The output is `yourname.predictive-buffer.geode`. Drop it into your Geometry Dash
`geode/mods` folder.

No local setup? Push this folder to a GitHub repo and run the **Build Mod**
workflow (Actions tab). Download the artifact, which contains the `.geode` file.

## Heads-up

The code was written against the Geode bindings but has not been compiled yet.
Field names (`m_isOnGround`, `m_touchingRings`, `m_isTestMode`, etc.) and
function signatures can vary between versions. If the compiler complains,
check the name against your SDK's bindings.
