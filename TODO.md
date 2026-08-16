# TODO — live remaining work

Updated 2026-07-17 (Windows-oracle byte-exactness campaign).
This file is intentionally a short list of unfinished work.  Completed
campaigns, the retired tooling, and the burn-down documentation all live
in git history (≤ 2026-07-15).

## 0. Push the reccmp fork, flip the pin  ⬅ BLOCKS FRESH CLONES

The LE + Watcom-debug parsers were migrated onto the reccmp fork
(commits `d0a743c5` + `e9bb551f` on its `watcom-reconstruction` branch,
still local-only).  Until those are pushed, `pyproject.toml` sources
reccmp from the sibling `../reccmp` checkout — a fresh clone's
`uv sync` will fail.  To finish:

```bash
cd ../reccmp && git push origin watcom-reconstruction
# then in this repo's pyproject.toml: delete the temp local-path line,
# uncomment the staged git pin (e9bb551f…), and run:
uv lock && uv sync && uv run pytest
```

## Current verified state

- reccmp (`c2 reccmp code` / `data`): **2234/2234 functions implemented and
  address-aligned at 100% accuracy; 1593 initialized-data symbols, 0
  issues**.
- Final-link comparison (`c2 rebuild`, every line exact):
  - game C 1435/1435 · c2-asm 87/87 · av-delink 517/517 · crt 195/195;
  - initialized data 341/341; LE sizes all exact;
  - placement: code starts 2234/2234 exact; data placement 1538/1538 named
    exact, 58 statics via delink/anchor;
  - **strict whole-code-object: 0 differing bytes / 508368** with only
    loader fixups masked and every relative branch displacement visible.

The burn-down diagnostic toolkit was retired 2026-07-15 (see git history);
verification is `c2 rebuild` + the reccmp fork.  All figures above were
re-verified after the parser migration onto `reccmp.formats` (whole file
0 differing bytes / 1,304,734 incl. the regenerated debug trailer).

## 1. Header provenance, not data placement

Corroborate original header filenames / include graph and the 35 non-data
lib32 slots only if an external source artifact appears.  Do not sacrifice
exact BSS placement for an unsupported filename guess.

## 2. Oracle data reintroduction (eventually)

The Mac/Windows oracle data (windows func/global maps, crossbuild map,
flag-survey results) was removed with the diagnostic toolkit; copies exist
outside the repo and can be reintroduced if a use case appears.

## 3. Windows oracle: break the operand-order canonicalization residue

The dual-target campaign (2026-07-17) verifies each TU against
CAESAR2.EXE build A with MSVC 4.0 /Od (`c2 win-verify` in the tooling
repo).  Most non-exact functions fall to ordinary source-shape recovery;
what remains in the ground-down TUs is a single residue class.

**The class.**  For commutative operands (`a + b`, `a * b`) and
relational mirrors (`a < b` vs `b > a`), MSVC 4.0 /Od chooses which
operand to load first by consulting compiler-internal expression/
identifier-table state, not source order.  Watcom DOES follow source
order, so the spelling is pinned by the DOS bytes and cannot be used to
steer MSVC.  When MSVC's choice differs from build A's, one instruction
changes length (reg,mem vs mem,reg / shl vs add+lea) and every later
branch displacement shifts with it.

**Currently blocked sites** (each is ONE decision plus its ripple):

- `formulae.c`: random_event temple-weight sum (125 B — med*2 vs
  large*4 first), adjust_peace_criteria (31 B), adjust_culture_criteria
  (36 B), do_promotion (25 B), get_army_totals (1 B),
  get_temple_tip (2 B);
- `pump.c`: InsertNode `text_buf[p + i]` (2 B);
- `lib32.c`: totalXpercentX100 imul order (2 B).

**Levers proven NOT to move these sites** (all tested on
random_event/adjust_peace, 2026-07-17): operand spelling and relational
mirroring; parenthesization; bool-assign vs if/else statement form;
`x*2` vs `2*x` vs `x+x`; extra extern declarations (new-name pads DO
move a different subclass — do_promotion/proserity responded — but not
these); renaming upstream locals (30-name sweep); sharing an upstream
local's identifier; fold-neutral expression-count edits (`+ 1 + 0`).
One working observation: a mul node SHARED with an earlier expression
in the TU (random_event's `large*4` also appears in its event-0 sum)
sits on the losing side, suggesting a fresh-vs-stale node rule — but
the same sharing exists in build A's source, so the rule must depend on
table state that diverged further upstream, beyond anything reachable
from this TU's text.

**What is missing to break it: the actual rule.**  Reverse-engineer the
canonicalization from MSVC 4.0's compiler itself:

1. extract the C compiler pass (C1.EXE / C1XX or the combined CL) from
   the `localhost/msvc-4.00-wibo` toolchain image in the tooling repo;
2. locate the expression-tree construction for binary commutative ops
   and the relational emitter — find the comparison that picks the
   first-loaded operand (likely an id/pointer compare on hash-consed
   nodes, given the observed insensitivity to names and counts);
3. model node-id assignment well enough to predict the choice from a
   TU prefix; validate the model against the ~40 already-exact
   formulae/pump functions (they constrain it tightly);
4. then either (a) find legitimate source text whose node history
   produces build A's choices, or (b) prove the divergence is caused by
   genuinely different Windows-era upstream source and classify those
   sites as per-build differences with a guard.

Secondary attacks if the disassembly route stalls: fully characterize
the decision function empirically (differential fuzzing of minimal TUs
against the site outcome), or bring in a second Windows build
(build B) as another witness to separate compiler state from source.

**Supporting fact worth keeping:** MSVC 4.0 assigns frame slots by
identifier-table order, so build A also fixes each function's local-name
hash layout.  The cohort/slave functions were made byte-exact by
searching names against that layout (reconstruction commit `47cc5bb`);
those names are witnesses, not recovered truth.  The same search
technique (bulk rank probes + per-function slot readers driving
`c2.win_bytes`) applies to every remaining slot-noise function and is
reconstructable from the tooling repo's `scripts/wv_check.py` /
`scripts/wv_tu.py` plus git history around caesar2 `9bd87bbf`.

## Explicitly out of scope

- byte-exact Watcom debug section;
- original source file/path strings and file naming;
- inverse-compiler research.
