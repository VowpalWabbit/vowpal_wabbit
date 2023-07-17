def binary_classification_one_feature(input_vector):
    if input_vector[0] > 0.5:
        return 2
    return 1


def multi_classification_two_features(input_vector):
    # Define the number of divisions for each feature
    divisions = 5

    # Calculate the division size for each feature
    division_size = 1 / divisions

    # Calculate the class index based on the input vector's position in the feature space
    class_idx = int(input_vector[0] // division_size) * divisions + int(
        input_vector[1] // division_size
    )

    return class_idx + 1
