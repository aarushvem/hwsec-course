import json
import numpy as np

from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report
from sklearn.model_selection import train_test_split


def eval():
    y_pred_full, y_test_full = [], []

    # 1. Load data from traces file
    with open("part2/traces.out", "r") as f:
        data = json.load(f)

    traces = data["traces"]
    labels = data["labels"]

    # Convert traces to feature vectors (simple + effective)
    X = np.array([
        np.mean(t) if len(t) > 0 else 0 for t in traces
    ]).reshape(-1, 1)

    y = np.array(labels)

    # Re-train 10 times to reduce randomness
    for i in range(10):
        # 2. Split data
        X_train, X_test, y_train, y_test = train_test_split(
            X,
            y,
            test_size=0.2,
            random_state=i,
            stratify=y
        )

        # 3. Train classifier
        clf = RandomForestClassifier(
            n_estimators=200,
            random_state=i
        )
        clf.fit(X_train, y_train)

        # 4. Predict on test set
        y_pred = clf.predict(X_test)

        # Do not modify the next two lines
        y_test_full.extend(y_test)
        y_pred_full.extend(y_pred)

    # 5. Print classification report
    print(classification_report(y_test_full, y_pred_full))


if __name__ == "__main__":
    eval()
