# Branch Protection Settings for GitHub Repository
# This file contains the recommended branch protection rules
# Repository administrators can apply these settings via GitHub UI or API

## Recommended Branch Protection Rules for 'main' branch:

### Required Status Checks:
- ✅ Require status checks to pass before merging
- ✅ Require branches to be up to date before merging
- Required checks:
  - `test (ubuntu-latest)`
  - `test (macos-latest)`  
  - `build-check`
  - `code-quality`

### Pull Request Requirements:
- ✅ Require a pull request before merging
- ✅ Require approvals: 1
- ✅ Dismiss stale reviews when new commits are pushed
- ✅ Require review from code owners (if CODEOWNERS file exists)

### Additional Restrictions:
- ✅ Restrict pushes that create files larger than 100MB
- ✅ Require linear history (optional, prevents merge commits)
- ✅ Allow force pushes: Disabled
- ✅ Allow deletions: Disabled

### Admin Enforcement:
- ✅ Include administrators in these restrictions

## GitHub CLI Commands to Apply Settings:

```bash
# Enable branch protection with required status checks
gh api repos/aylabs/SddGtk/branches/main/protection \
  --method PUT \
  --field required_status_checks='{"strict":true,"contexts":["test (ubuntu-latest)","test (macos-latest)","build-check","code-quality"]}' \
  --field enforce_admins=true \
  --field required_pull_request_reviews='{"required_approving_review_count":1,"dismiss_stale_reviews":true}' \
  --field restrictions=null
```

## Manual Setup via GitHub UI:
1. Go to repository Settings → Branches
2. Click "Add rule" for main branch
3. Configure the settings listed above
4. Save the protection rule