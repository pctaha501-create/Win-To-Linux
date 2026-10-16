# AI Screen Share: setup, flow, and security

## User flow

1. Open **AI Screen Share** from the Win-To-Linux sidebar or Start search (`screen share` / `MCP`).
2. Press **Create MCP**. Win-To-Linux sends `POST https://screen-share.the-end.shop/api/sessions` and validates the returned HTTPS host/viewer URLs before displaying them.
3. Press **Open host page** on the computer whose screen will be shared.
4. In the host browser, press **Share screen** and explicitly select the screen, window, or tab. The browser owns the permission prompt; Win-To-Linux does not bypass it.
5. Add `https://mcp.the-end.shop/mcp` to an AI client that supports remote MCP and complete its normal OAuth sign-in.
6. Press **Copy AI connection details** and give those details only to the intended AI client. The current snapshot tool is `get_screen_snapshot`, with the session ID and viewer token.
7. Stop sharing from the host browser when done.

## Service architecture

- Desktop UI: Qt Quick page plus `ScreenShareController`.
- Session broker and WebRTC signaling: `https://screen-share.the-end.shop`.
- Remote MCP transport and OAuth: `https://mcp.the-end.shop/mcp`.
- Screen-share service generates separate host/viewer tokens. The viewer token is used to authorize status/snapshot requests for that session.

## Security boundaries

- The desktop client accepts only HTTPS links on the expected `screen-share.the-end.shop` host, and opens the MCP endpoint only on `mcp.the-end.shop`.
- Creating a session does not start capture. The host browser must explicitly grant screen capture.
- Session links and viewer tokens are credentials. Do not post them in public chats or issue trackers.
- This integration grants view/snapshot access only. It does not implement remote mouse/keyboard control.
- OAuth setup remains the responsibility of the AI client; the app does not claim to auto-configure a client.

## Verification

The Qt shell smoke test checks that the new navigation entry and **Create MCP** control are present. CI build and tests must pass before treating the change as integrated. The live session endpoint can be checked with:

```sh
curl -fsS https://screen-share.the-end.shop/health
```

Do not log the JSON response from `POST /api/sessions`, because it contains private host/viewer credentials.
