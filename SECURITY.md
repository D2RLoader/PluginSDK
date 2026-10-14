# Security Policy

This page explains how to report a security problem in the D2RLoader Plugin SDK. Please read it before you report.

## Report a problem

Use GitHub private reporting. This keeps your report private until we publish a fix.

1. Open the [Security tab](https://github.com/D2RLoader/PluginSDK/security) of this repository.
2. Select **Report a vulnerability**.
3. Fill in the form. Add the details listed in "What to include".

If you cannot use the form, open a public issue titled "Security contact request". Do not write any details in the issue. We will then give you a private way to contact us.

Please do not report security problems in public issues, pull requests, or chat rooms.

## What a plugin can do

A plugin is a native DLL. It runs inside the game process with the same rights as the game. Treat it like any program that you install.

Install plugins only from sources that you trust. A plugin that does harm on purpose is not a vulnerability in the SDK.

## What is in scope

Please report a flaw when it fits one of these cases:

- **SDK files.** A header in `include/D2RLPlugin`, a build script in `cmake/`, or an example in `examples/` has a flaw. Developers copy examples, so an unsafe example counts.
- **Data from outside.** Data that a plugin does not control can cause a memory error or break a documented limit. This data includes messages from other players, HTTP responses, and resource files.
- **Service checks.** A documented check does not catch bad input. Examples are a size check, a handle check, and an ABI version check.
- **D2RLoader itself.** D2RLoader has a flaw that you can reach through an SDK service.

## What is out of scope

- A plugin from another author. Report it to that author.
- A plugin that does harm on purpose. See "What a plugin can do".
- A bug in Diablo II: Resurrected or on Blizzard servers. Report it to Blizzard.
- A normal bug that no other person can cause, such as a crash in your own plugin. Open a normal public issue.

## Test safely

- Test only on your own computer, with your own copy of the game.
- Use plugins that you built or have permission to test.
- Test multiplayer problems with two game copies that you control. Do not test on other players or on public servers.
- Do not read, change, or delete data that belongs to other people.

We welcome good-faith research that follows these rules.

## What to include

- The SDK version (`D2RL_SDK_VERSION`) and the plugin ABI version (`D2RL_PLUGIN_ABI_VERSION`). Both are in `include/D2RLPlugin/version.h`.
- The D2RLoader version, if you know it.
- What happens, and what you expected to happen.
- Steps to reproduce the problem. A small plugin or code sample helps.
- The impact. For example, who can cause the problem and what it can damage.
- How you want us to credit you. We use your GitHub name unless you ask for another name or for no credit.

## What happens next

1. We reply within 7 days to tell you that we received your report.
2. We tell you if we accept it as a vulnerability. We may ask you questions.
3. We fix the problem and release a new SDK version.
4. We publish a GitHub security advisory. We credit you in it, unless you ask us not to.

Please keep the details private until we publish the advisory. If we have not published a fix 90 days after your report, you may publish your findings.

## Supported versions

We fix security problems in the latest SDK release only. Before you report, check that the problem still exists in the latest release.
