#!/usr/bin/perl

my %started;
my $mode = 1;

# Do nl on output first

while (<>)
{
    if ($mode == 1) {
        /^\s*\d+\s+\d\s+(\w).*started/ && do {
            exists $started{$1}
                and print "Hash key already exists:\n", $started{$1}, $_;
            $started{$1} = $_;
        };
        /^\s*\d+\s+\d\s+(\w).*(exited|cancelling)/ && do {
            delete $started{$1};
        };
    } elsif ($mode == 2) {
        /^.*new ClassMuxer at channel (\d+)/ && do {
            exists $started{$1}
                and print "Hash key already exists:\n", $started{$1}, $_;
            $started{$1} = $_;
        };
        /^.*shutting down channel (\d+)/ && do {
            delete $started{$1};
        };
    }
}

print "Not paired:\n";
print %started;