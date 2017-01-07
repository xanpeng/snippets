# Path to your oh-my-fish.
set fish_path $HOME/.oh-my-fish
 
# Theme
# set fish_theme uggedal
set fish_theme robbyrussell
 
set -x PATH $PATH /usr/local/go/bin
set -x GOPATH ~/gopath/
 
# Which plugins would you like to load? (plugins can be found in ~/.oh-my-fish/plugins/*)
# Custom plugins may be added to ~/.oh-my-fish/custom/plugins/
# Example format: set fish_plugins autojump bundler
 
# Path to your custom folder (default path is $FISH/custom)
#set fish_custom $HOME/dotfiles/oh-my-fish
 
# Load oh-my-fish cofiguration.
. $fish_path/oh-my-fish.fish
 
# bind \e. 'commandline -i -- (echo $history[1] | rev | cut -d " " -f1 | rev)'
function fish_user_key_bindings
    bind \e. 'history-token-search-backward'
end
