
git clone git@github.com:Real-Time-MIMO/RadarPipeline.git
git submodule init
git submodule update


####### FIREWALL ########

# sudo ufw default allow outgoing
# sudo ufw default deny incoming

# TCP Connection firewall
sudo ufw allow 8080/tcp

# SSH Connection firewall
sudo ufw allow 491/tcp

# Export FilePath
LD_LIBRARY_PATH=##########
export LD_LIBRARY_PATH
