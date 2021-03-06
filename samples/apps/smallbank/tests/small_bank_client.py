# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.
import perfclient
import sys
import os

if __name__ == "__main__":

    def add(parser):
        parser.add_argument(
            "-u", "--accounts", help="Number of accounts", default=10, type=int
        )

    args, unknown_args = perfclient.cli_args(add=add, accept_unknown=True)

    unknown_args = [term for arg in unknown_args for term in arg.split(" ")]

    def get_command(*common_args):
        return [*common_args, "--accounts", str(args.accounts)] + unknown_args

    args.package = "libsmallbankenc"
    perfclient.run(args.build_dir, get_command, args)
