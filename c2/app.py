"""Main typer application for the c2 toolkit."""

import os as _os

# Plain-text output: the c2 tools emit NO ANSI colour by default -- cleaner
# for piping, logs, diffing, and agent consumption.  Rich honours NO_COLOR
# (it still parses `[markup]` so tags are consumed, just no colour codes are
# emitted).  Set this BEFORE any command module is imported so every Console
# in the process picks it up.  Opt back in with `C2_COLOR=1`.
if _os.environ.get("C2_COLOR", "").lower() not in ("1", "true", "yes"):
    _os.environ.setdefault("NO_COLOR", "1")

import typer

from c2.commands.delink import delink
from c2.commands.rebuild import rebuild
from c2.commands.reccmp import app as reccmp_app
from c2.commands.fetch_original import fetch_original
from c2.commands.win_verify import win_verify

app = typer.Typer(
    name="c2",
    help="Caesar II reconstruction toolkit: build the recovered PS.EXE and verify it.",
    no_args_is_help=True,
)

app.command("fetch-original")(fetch_original)
app.command("delink")(delink)
app.command("rebuild")(rebuild)
app.command("win-verify")(win_verify)
app.add_typer(reccmp_app, name="reccmp")
