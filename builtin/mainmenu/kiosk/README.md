# Luanti demo mode

## Set up

* Replace the Luanti mainmenu with this one. This could be done by direct file replacement, checking out the kiosk branch, or placing it somewhere and using `mainmenu_path`
* Update minetest.conf

```conf
# Username and password, should be unique per machine
name = fosdem
fosdem_password = password

# Other settings, tweak as needed
enable_esc_dialog = false
font_size = 16
gui_scaling = 1.2
```
