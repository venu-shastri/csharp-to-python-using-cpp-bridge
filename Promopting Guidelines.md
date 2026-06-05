# Prompting Guidelines

### Review only DependencyResolver.cs

Focus on:
- Memory leaks
- Thread safety

Ignore:
- Naming
- Formatting


Technology:
- .NET 8

Architecture:
- Dependency Injection
- Clean Architecture

Testing:
- xUnit
- FluentAssertions

Logging:
- ILogger

Avoid:
- Static state
- Service Locator


skills/
├── perform-code-review.md
├── generate-unit-tests.md
├── analyze-technical-debt.md
├── review-security-risks.md
├── create-impact-analysis.md
├── review-api-design.md
└── assess-breaking-changes.md



# Skill: Code Review

Review the code for:

- SOLID principles
- Thread safety
- Exception handling
- Resource leaks
- Security concerns

Ignore:

- Formatting
- Naming

Return:

1. Findings
2. Risk Level
3. Recommendation
