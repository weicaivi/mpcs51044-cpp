#include <iostream>
#include <string>
#include <memory>

class Node {
private:
    std::unique_ptr<Node> cloneTree() const {
        auto new_node = std::make_unique<Node>(data, isQuestion);
        if (yes) new_node->yes = yes->cloneTree();
        if (no) new_node->no = no->cloneTree();
        return new_node;
    }

public:
    std::string data;
    std::unique_ptr<Node> yes;
    std::unique_ptr<Node> no;
    bool isQuestion;

    Node(const std::string& d, bool q) : data(d), isQuestion(q) {}
    friend class AnimalGame;
};

class AnimalGame {
private:
    std::unique_ptr<Node> root;
    std::unique_ptr<Node> initialTree;

    std::string getLine(const std::string& prompt) {
        std::string input;
        std::cout << prompt;
        std::getline(std::cin, input);
        return input;
    }

    bool getYesNo(const std::string& prompt) {
        while (true) {
            std::string response = getLine(prompt + " (y/n): ");
            if (response == "y" || response == "yes") return true;
            if (response == "n" || response == "no") return false;
            std::cout << "Please answer yes or no.\n";
        }
    }

    void learn(Node* current, const std::string& newAnimal) {
        std::cout << "You won!\n";
        std::string newQuestion = getLine("Enter a yes/no question that distinguishes a " + 
                                        newAnimal + " from a " + current->data + ": ");

        bool answerForNew = getYesNo("For a " + newAnimal + ", what's the answer?");

        auto oldAnimal = std::make_unique<Node>(current->data, false);
        auto newAnimalNode = std::make_unique<Node>(newAnimal, false);

        current->isQuestion = true;
        current->data = newQuestion;

        if (answerForNew) {
            current->yes = std::move(newAnimalNode);
            current->no = std::move(oldAnimal);
        } else {
            current->yes = std::move(oldAnimal);
            current->no = std::move(newAnimalNode);
        }
    }

    void saveInitialState() {
        initialTree = root->cloneTree();
    }

public:
    AnimalGame() {
        root = std::make_unique<Node>("Does it fly?", true);
        root->yes = std::make_unique<Node>("parrot", false);
        root->no = std::make_unique<Node>("cat", false);
        saveInitialState();
    }

    void play() {
        std::cout << "Think of an animal...\n";
        Node* current = root.get();

        while (true) {
            if (current->isQuestion) {
                if (getYesNo(current->data)) {
                    current = current->yes.get();
                } else {
                    current = current->no.get();
                }
            } else {
                bool correct = getYesNo("Is it a " + current->data + "?");
                if (correct) {
                    std::cout << "I win!\n";
                    return;
                } else {
                    std::string newAnimal = getLine("What animal were you thinking of? ");
                    learn(current, newAnimal);
                    return;
                }
            }
        }
    }

    void run() {
        while (true) {
            std::string command = getLine("\nEnter command (play/forget/quit): ");
            if (command == "play") play();
            else if (command == "forget") forget();
            else if (command == "quit") break;
            else std::cout << "Invalid command.\n";
        }
    }

    void forget() {
        root = initialTree->cloneTree();
        std::cout << "Memory reset to initial state.\n";
    }
};

int main() {
    AnimalGame game;
    game.run();
    return 0;
}