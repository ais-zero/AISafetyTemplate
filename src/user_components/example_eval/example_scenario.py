"""Custom HELM Scenario with hardcoded multiple-choice Q&A pairs."""

from typing import List

from helm.benchmark.scenarios.scenario import (
    Scenario,
    Instance,
    Input,
    Reference,
    Output,
    CORRECT_TAG,
    TRAIN_SPLIT,
    TEST_SPLIT,
)

# Hardcoded Q&A pairs: (question, correct_answer, [distractors])
QA_PAIRS = [
    {
        "question": "What is the capital of France?",
        "correct": "Paris",
        "distractors": ["London", "Berlin", "Madrid"],
    },
    {
        "question": "Which planet is known as the Red Planet?",
        "correct": "Mars",
        "distractors": ["Venus", "Jupiter", "Saturn"],
    },
    {
        "question": "What is the chemical symbol for water?",
        "correct": "H2O",
        "distractors": ["CO2", "NaCl", "O2"],
    },
    {
        "question": "Who wrote 'Romeo and Juliet'?",
        "correct": "William Shakespeare",
        "distractors": ["Charles Dickens", "Jane Austen", "Mark Twain"],
    },
    {
        "question": "What is the largest mammal on Earth?",
        "correct": "Blue whale",
        "distractors": ["African elephant", "Giraffe", "Polar bear"],
    },
    {
        "question": "In what year did World War II end?",
        "correct": "1945",
        "distractors": ["1939", "1944", "1950"],
    },
    {
        "question": "What is the speed of light in vacuum approximately?",
        "correct": "300,000 km/s",
        "distractors": ["150,000 km/s", "500,000 km/s", "1,000,000 km/s"],
    },
    {
        "question": "Which element has the atomic number 1?",
        "correct": "Hydrogen",
        "distractors": ["Helium", "Lithium", "Carbon"],
    },
    {
        "question": "What is the square root of 144?",
        "correct": "12",
        "distractors": ["10", "14", "16"],
    },
    {
        "question": "Which organ pumps blood throughout the human body?",
        "correct": "Heart",
        "distractors": ["Liver", "Brain", "Lungs"],
    },
]

OPTION_LABELS = ["A", "B", "C", "D"]


class ExampleQAScenario(Scenario):
    """A simple multiple-choice Q&A scenario with hardcoded questions."""

    name = "example_qa"
    description = "Hardcoded multiple-choice Q&A for testing HELM in a sandbox."
    tags = ["knowledge", "multiple_choice"]

    def get_instances(self, output_path: str) -> List[Instance]:
        instances: List[Instance] = []

        for i, qa in enumerate(QA_PAIRS):
            all_options = [qa["correct"]] + qa["distractors"]

            references: List[Reference] = []
            for j, option in enumerate(all_options):
                tags = [CORRECT_TAG] if option == qa["correct"] else []
                references.append(
                    Reference(
                        output=Output(text=option),
                        tags=tags,
                    )
                )

            split = TRAIN_SPLIT if i < 5 else TEST_SPLIT

            instance = Instance(
                input=Input(text=qa["question"]),
                references=references,
                split=split,
            )
            instances.append(instance)

        return instances
