# AI Agent Cost Optimization Playbook

## Purpose

This guide provides practical techniques for working effectively with GitHub Copilot, Copilot Chat, Cursor, Claude Code, Cline, Codex, and other AI coding agents while minimizing token consumption, reducing iterations, and improving response quality.

---

# 1. Use `/clear` Frequently

After completing a task:

```text
/clear
```

This prevents the agent from carrying irrelevant context into future requests.

### Benefits

* Lower token usage
* Faster responses
* Less confusion

---

# 2. Start With a Precise Goal

### Avoid

```text
Fix this code.
```

### Prefer

```text
Analyze this code for thread-safety issues only.
Do not suggest style improvements.
```

### Benefits

* Smaller responses
* Reduced reasoning overhead
* Lower cost

---

# 3. Limit Scope Explicitly

### Avoid

```text
Review the repository.
```

### Prefer

```text
Review only the DependencyResolver class.
Focus on SOLID violations.
Ignore naming conventions.
```

### Benefits

* Less context loaded
* More focused answers

---

# 4. Tell the Agent What Not to Do

Examples:

```text
Generate only the implementation.
Do not explain.
Do not generate tests.
```

or

```text
Return only the diff.
```

### Benefits

* Significant reduction in output tokens

---

# 5. Use Multi-Step Conversations

### Avoid

```text
Analyze architecture, suggest improvements,
generate code, generate tests,
generate documentation.
```

### Prefer

```text
Step 1: Analyze architecture only.

Step 2: Generate implementation.

Step 3: Generate tests.
```

### Benefits

* Improved accuracy
* Easier validation
* Lower rework cost

---

# 6. Reference Specific Files

### Avoid

```text
Find where XML validation happens.
```

### Prefer

```text
Look only in:
- XmlValidator.cs
- SchemaLoader.cs

Find XML validation logic.
```

### Benefits

* Prevents repository-wide searches
* Saves tokens

---

# 7. Create Reusable Prompt Templates

Example:

```text
Task:
Review code for:

1. Thread safety
2. Resource leaks
3. Exception handling

Ignore:
- Naming
- Formatting
- Documentation
```

### Benefits

* Consistent reviews
* Reduced prompt-writing effort

---

# 8. Ask for Assumptions First

### Instead of

```text
Implement plugin architecture.
```

### Use

```text
Before implementation,
list assumptions and risks.
```

### Benefits

* Avoids expensive rewrites

---

# 9. Request Structured Output

Examples:

```text
Return output in:

Issue
Impact
Recommendation
```

or

```text
Output JSON only.
```

### Benefits

* Easier automation
* Fewer follow-up prompts

---

# 10. Use Repository Instructions

Create:

```text
.github/copilot-instructions.md
```

Example:

```text
Technology:
- .NET 8
- C#
- xUnit

Architecture:
- Clean Architecture
- Dependency Injection

Rules:
- No static services
- Use async/await
- Use ILogger
```

### Benefits

* Eliminates repeated instructions
* Consistent output

---

# 11. Avoid Large File Dumps

### Avoid

```text
Analyze these 5000 lines.
```

### Prefer

```text
Here is the relevant method.
Focus on memory allocation.
```

### Benefits

* Major token savings

---

# 12. Use "Act As" Roles

Example:

```text
Act as a .NET architect.

Review dependency management strategy.

Focus only on versioning and package governance.
```

### Benefits

* Narrows reasoning scope
* Produces more relevant answers

---

# 13. Ask for a Plan Before Code

### Avoid

```text
Implement feature X.
```

### Prefer

```text
Provide implementation plan.
Wait for approval before coding.
```

### Benefits

* Prevents unnecessary code generation
* Encourages alignment before implementation

---

# 14. Leverage Agent Modes Correctly

## Explore Mode

```text
Find where package versions are resolved.
```

## Plan Mode

```text
Design a solution for centralized dependency management.
```

## Execute Mode

```text
Implement the approved design.
```

### Benefits

* Better task decomposition
* Reduced wasted iterations

---

# 15. Keep Context Local

Instead of relying on chat history, create source-of-truth documents:

```text
ARCHITECTURE.md
DEPENDENCY_RULES.md
ENGINEERING_GUIDELINES.md
```

### Benefits

* Better reproducibility
* Reduced dependency on conversation context

---

# 16. Ask for Diffs Instead of Full Files

### Avoid

```text
Regenerate the entire class.
```

### Prefer

```text
Show only changed methods.
```

or

```text
Generate git diff.
```

### Benefits

* 80–90% reduction in output size
* Easier code review

---

# 17. Use Acceptance Criteria

Example:

```text
Implement:

Acceptance Criteria:
- Supports .NET 8
- Works on Linux
- No breaking changes
- Unit test coverage >80%
```

### Benefits

* Clear expectations
* Better implementation quality

---

# 18. Refactor Incrementally

### Avoid

```text
Refactor the entire repository.
```

### Prefer

```text
Refactor only the DependencyResolver.
After approval, continue with PackageManager.
```

### Benefits

* Reduced risk
* Easier validation
* Better control over changes

---

# Golden Rule

Before sending any prompt, ask:

> Can I reduce the scope, context, or output size by 50%?

The highest-impact optimization techniques are:

1. Use `/clear` regularly
2. Limit file scope
3. Ask for plans before code
4. Request diffs instead of full files
5. Use repository instructions (`copilot-instructions.md`)
6. Break large tasks into smaller steps

Following these practices consistently will improve response quality, reduce token usage, and lower overall AI-assisted development costs.
