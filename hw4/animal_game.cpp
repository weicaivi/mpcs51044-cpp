#include <iostream>
#include <string>
#include <memory>
using namespace std;
class State;

class Node {
public:
	Node(string name);
    Node(unique_ptr<State> s) : state(move(s)) {}
	void reset(string name);
    void process();
    unique_ptr<State> state;
};

class State {
public:
    virtual void process(Node &node) = 0;
    virtual ~State() = default; // Designed for inheritance
};

class QuestionState  : public State {
public:
    QuestionState(string ques, unique_ptr<Node> yes, unique_ptr<Node> no);
    virtual void process(Node &node) override;
private:
    string question;
    unique_ptr<Node> yesNode;
    unique_ptr<Node> noNode;
};

class AnswerState : public State {
public:
	AnswerState(string a) : animal(a) {}
    virtual void process(Node &node) override;
private:
    string animal;
};

Node::Node(string name) : state(make_unique<AnswerState>(name)) {}

void Node::reset(string name)
{
	state = make_unique<AnswerState>(name);
}

void
Node::process()
{
    state->process(*this);
}

QuestionState::QuestionState(string q, unique_ptr<Node> y, unique_ptr<Node> n)
  : question(q), yesNode(move(y)), noNode(move(n)) {}

void
QuestionState::process(Node &node)
{
    string answer;
    cout << question << " ";
    getline(cin, answer);
    if(answer == "y" || answer == "yes" || answer == "Yes") {
        yesNode->process();
    } else {
        noNode->process();
    }
}

void
AnswerState::process(Node &node)
{
    string answer;
    cout << "Are you thinking of a " << animal << "? ";
    // cin >> answer;  // Don't mix formatted and unformatted I/O
    getline(cin, answer);
    if(answer == "y" || answer == "yes" || answer == "Yes") {
        cout << "I knew it!" << endl;
    } else {
        string newAnimal;
        cout << "Gee, you stumped me, "
             << "what were you thinking of? ";
        getline(cin, newAnimal);
        cout << "What is a question to distinguish a "
             << newAnimal << " from a " << animal << "? ";
        string newQuestion;
        getline(cin, newQuestion);
        node.state
          = make_unique<QuestionState>
                 (newQuestion,
                  make_unique<Node>(newAnimal),
                  make_unique<Node>(move(node.state)));
    }
}

int
main()
{
	Node root{"butterfly"};
    cout << "Let's play the animal game" << endl;
    for(;;) {
        cout << "Think of an animal" << endl;
        root.process();
		cout << "That was fun. Do your want to play again? y(yes)/n(no)/r(reset data)";
		string response;
		getline(cin, response);
		if (response == "y")
			continue;
		if (response == "n")
			return 0;
		root.reset("butterfly");
	}
}
