from typing import Dict, List, Optional


class TestData:
    id: int
    depends_on: List[int]
    command_line: str
    # Helper member for flatbuffer conversion
    stashed_command_line: Optional[str] = None
    is_shell: bool
    input_files: List[str]
    comparison_files: Dict[str, str]
    skip: bool
    skip_reason: Optional[str]

    def __init__(
        self,
        id,
        description,
        depends_on,
        command_line,
        is_shell,
        input_files,
        comparison_files,
        skip,
        skip_reason,
    ):
        self.id = id
        self.description = description
        self.depends_on = depends_on
        self.command_line = command_line
        self.is_shell = is_shell
        self.input_files = input_files
        self.comparison_files = comparison_files
        self.skip = skip
        self.skip_reason = skip_reason
