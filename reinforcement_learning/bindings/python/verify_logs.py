import argparse
import json
import sys
from collections import defaultdict

def process_cmd_line(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('--remote_log_file', help='client json config', required=True)
    parser.add_argument('--log_file', help='log file that was replayed', required=True)
    return parser.parse_args(args)

def main(args):
    options = process_cmd_line(args)

    # Due to replaying logs on the same loop after the EventHub buffer has cleared, EventIds can be duplicated.
    # Use lists to deal with this instead of assuming EventId is unique.
    remote_logged_events = defaultdict(list)

    # Preprocess and save the events from the remote log into a dictionary for easy lookup
    with open(options.remote_log_file) as fp:
        for count, line in enumerate(fp):
            current_example = json.loads(line)
            remote_logged_events[current_example["EventId"]].append(current_example)

    success = 0
    fail = 0
    # Iterate over each of the events that should have been sent to the server and verify they were received.
    with open(options.log_file) as fp:
        for count, line in enumerate(fp):
            current_example = json.loads(line)
            current_event_id = current_example["EventId"]
            passed = True

            # Verify the example that was sent exists in the remote log.
            if(current_event_id not in remote_logged_events):
                print("Err: " + current_event_id + " not found in remote log")
                passed = False

            # Since the remote log may contain duplcates, check over each.
            for remote_example in remote_logged_events[current_event_id]:
                # Verify that the context that was sent matches.
                if(remote_example["c"] != current_example["c"]):
                    print("Err: context does not match for " + current_event_id)
                    passed = False

                # Verify that the observations that were sent match.
                if(("o" in current_example) and (("o" not in  remote_example) or (remote_example["o"] != current_example["o"]))):
                    print("Err: observations do not match for " + current_event_id)
                    passed = False

            if(passed):
                success = success + 1
            else:
                fail = fail = 1

    print("Pass: " + str(success))
    print("Fail: " + str(fail))


if __name__ == "__main__":
   main(sys.argv[1:])
