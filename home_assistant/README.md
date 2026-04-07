# Rowing Data In Home Assistant (ESPHome Native API)

This project already publishes rowing entities through the ESPHome native API. Use this quick setup to get stable ingestion and a usable dashboard.

## 1) ESPHome node config

Use `rower-esp32.yaml` from project root. It already includes:

- `api:` enabled for Home Assistant discovery
- `rowing_monitor:` with all sensors and phase text sensor
- `external_components` pointing to this repository

The node now also uses:

- `api.reboot_timeout: 0s` to avoid API disconnect reboot loops

## 2) Home Assistant entity check

After flashing/restarting the node and reloading ESPHome integration, verify these entities exist:

- `sensor.rower_valid_strokes`
- `sensor.rower_short_strokes`
- `sensor.rower_micro_strokes`
- `sensor.rower_spm`
- `sensor.rower_active_time`
- `sensor.rower_distance`
- `sensor.rower_short_distance`
- `sensor.rower_micro_distance`
- `sensor.rower_avg_valid_travel`
- `sensor.rower_avg_short_travel`
- `sensor.rower_avg_micro_travel`
- `sensor.rower_phase`

## 3) Optional helpers

Add `home_assistant/rowing_helpers.yaml` into your HA `configuration.yaml` via packages/templates (or copy its blocks into your existing template config).

Helpers included:

- `binary_sensor.rower_session_active`
- `sensor.rower_total_strokes`
- `sensor.rower_session_distance`

If your entity IDs differ, update names in `rowing_helpers.yaml`.

## 4) Dashboard card

Add a manual card in Lovelace and paste `home_assistant/rowing_dashboard.yaml`.

If entity IDs differ, update them in the card YAML and helper file.

## 5) Troubleshooting

- No entities in HA: ensure ESPHome integration is connected and the node is online.
- Build succeeds but no live changes: check encoder wiring/pins and serial logs for `rowing_monitor` startup and stroke logs.
- Too many logs: this component now publishes sensor values only when values change, which should significantly reduce repeated sensor log lines.