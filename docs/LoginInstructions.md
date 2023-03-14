# Login Instructions

Since the all of the USB ports are being used in the current node setup, we suggest doing all development remotely using SSH. Specifically, using [Visual Studio Code](https://code.visualstudio.com/) opn your personal computer due to it's helpful extensions. If you are outside of the Naples EE Capstone Lab (and do not have access to the `ECE-EELAB-5.8GHz` WiFi network), go to step #1. If you are in the lab, connect to `ECE-EELAB-5.8GHz` and skip step #1. These steps assume that the nodes are connected to the WiFi network.

## Step #1: Remotely Access `eelabgpu1`

There is a computer in the Naples lab called `eelabgpu1`. It currently is running CentOS Linux Version 7 and has a TITAN X GPU. It is very difficult to get CUDA drivers to work so we do not reccomend using it. By remotely accessing this machine (from outside of the lab) you gain access to the local network which the nodes are connected to. Follow these steps:

1. Install [Visual Studio Code](https://code.visualstudio.com/) on your personal computer.
2. Add the "Remote - SSH" extension to VS Code. Other useful extensions might be suggested later.
3. Look at your SSH configuration file (might be located at `.ssh/config`) and add an entry that looks something like this:
    ```
    Host 128.111.57.75
        HostName 128.111.57.75
        Port XXX
        User fusionsense
    ```
4. Use VS Code to remotely login to the lab computer and enter the password (in the Google Drive).
5. Confirm that the WiFi adapter is plugged into the back of the computer.

## Step 2: Access the individual Nodes
1. Use the following commands depending on which node you want to login to. SSH authentications keys should already be configured so no passowrd will be required.
    ```
    ssh fusionsense@xxx
    ssh fusionsense@xxx
    ssh fusionsense@xxx
    ```
    This is what the SSH configuration file at `/.ssh/config` should look like:
    ```
    Host XXX.XXX.X.XXX
        HostName XXX.XXX.X.XXX
        Port XXX
        User fusionsense

    Host XXX.XXX.X.XXX
        HostName XXX.XXX.X.XXX
        Port XXX
        User fusionsense

    Host XXX.XXX.X.XXX
        HostName XXX.XXX.X.XXX
        Port XXX
        User fusionsense
    ```
## Conclusion

You can now remotely access the radar nodes. Either use your personal computer as the radar network server or `eelabgpu1`. For speeding up login using SSH key-based authentication, follow [this tutorial](https://www.digitalocean.com/community/tutorials/how-to-configure-ssh-key-based-authentication-on-a-linux-server). For adding SSH keys for Github development, follow [this tutorial](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account).



